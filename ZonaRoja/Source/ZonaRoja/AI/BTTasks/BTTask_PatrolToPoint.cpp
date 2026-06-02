// BTTask_PatrolToPoint.cpp
// Implementación del movimiento de patrulla con espera y rotación aleatoria

#include "AI/BTTasks/BTTask_PatrolToPoint.h"
#include "AI/PMCCharacter.h"
#include "AI/PMCAIController.h"

#include "BehaviorTree/BehaviorTreeComponent.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "AIController.h"
#include "Navigation/PathFollowingComponent.h"

UBTTask_PatrolToPoint::UBTTask_PatrolToPoint()
{
	// Nombre mostrado en el editor del BehaviorTree
	NodeName = TEXT("Patrullar al Punto");

	// Habilitar tick para gestionar la espera y rotación
	bNotifyTick = true;
	bNotifyTaskFinished = true;
}

EBTNodeResult::Type UBTTask_PatrolToPoint::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	// Inicializar la memoria de la tarea
	FPatrolMemory* Memory = reinterpret_cast<FPatrolMemory*>(NodeMemory);
	if (Memory)
	{
		*Memory = FPatrolMemory(); // Reset completo
	}

	APMCAIController* AIController = Cast<APMCAIController>(OwnerComp.GetAIOwner());
	if (!AIController)
	{
		return EBTNodeResult::Failed;
	}

	APMCCharacter* PMCCharacter = Cast<APMCCharacter>(AIController->GetPawn());
	if (!PMCCharacter)
	{
		return EBTNodeResult::Failed;
	}

	// Verificar que hay puntos de patrulla definidos
	if (PMCCharacter->PatrolPoints.Num() == 0)
	{
		UE_LOG(LogTemp, Verbose, TEXT("[BTTask_PatrolToPoint] %s no tiene puntos de patrulla"),
			*PMCCharacter->GetName());
		return EBTNodeResult::Failed;
	}

	UBlackboardComponent* BBComp = OwnerComp.GetBlackboardComponent();
	if (!BBComp)
	{
		return EBTNodeResult::Failed;
	}

	// Obtener el índice actual del Blackboard
	int32 PatrolIndex = BBComp->GetValueAsInt(APMCAIController::BB_PatrolIndex);

	// Asegurar que el índice es válido (puede haberse corrompido)
	PatrolIndex = FMath::Clamp(PatrolIndex, 0, PMCCharacter->PatrolPoints.Num() - 1);
	PMCCharacter->CurrentPatrolIndex = PatrolIndex;

	// Obtener la posición del punto de patrulla actual
	const FPatrolPoint& CurrentPoint = PMCCharacter->PatrolPoints[PatrolIndex];
	FVector TargetLocation = CurrentPoint.Location;

	// Solicitar movimiento hacia el punto
	FAIMoveRequest MoveRequest;
	MoveRequest.SetGoalLocation(TargetLocation);
	MoveRequest.SetAcceptanceRadius(AcceptanceRadius);
	MoveRequest.SetUsePathfinding(true);
	MoveRequest.SetAllowPartialPath(false);

	FAIRequestID RequestID = AIController->MoveTo(MoveRequest);

	if (RequestID.IsValid())
	{
		if (Memory)
		{
			Memory->bMoveRequested = true;
			Memory->TargetLookYaw = PMCCharacter->GetActorRotation().Yaw;
		}

		UE_LOG(LogTemp, Verbose, TEXT("[BTTask_PatrolToPoint] %s → Punto %d en %s"),
			*PMCCharacter->GetName(), PatrolIndex, *TargetLocation.ToString());

		return EBTNodeResult::InProgress;
	}

	UE_LOG(LogTemp, Warning, TEXT("[BTTask_PatrolToPoint] No se pudo iniciar movimiento al punto %d"), PatrolIndex);
	return EBTNodeResult::Failed;
}

void UBTTask_PatrolToPoint::TickTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds)
{
	FPatrolMemory* Memory = reinterpret_cast<FPatrolMemory*>(NodeMemory);
	if (!Memory)
	{
		FinishLatentTask(OwnerComp, EBTNodeResult::Failed);
		return;
	}

	APMCAIController* AIController = Cast<APMCAIController>(OwnerComp.GetAIOwner());
	if (!AIController)
	{
		FinishLatentTask(OwnerComp, EBTNodeResult::Failed);
		return;
	}

	APMCCharacter* PMCCharacter = Cast<APMCCharacter>(AIController->GetPawn());
	if (!PMCCharacter)
	{
		FinishLatentTask(OwnerComp, EBTNodeResult::Failed);
		return;
	}

	if (PMCCharacter->PatrolPoints.Num() == 0)
	{
		FinishLatentTask(OwnerComp, EBTNodeResult::Failed);
		return;
	}

	// ---- FASE 1: Esperando a llegar al punto ----

	if (!Memory->bIsWaiting)
	{
		// Comprobar si llegó al destino
		UPathFollowingComponent* PathFollowComp = AIController->GetPathFollowingComponent();
		if (PathFollowComp)
		{
			EPathFollowingStatus::Type Status = PathFollowComp->GetStatus();

			if (Status == EPathFollowingStatus::Idle || Status == EPathFollowingStatus::Waiting)
			{
				// Llegó al punto de patrulla: iniciar espera
				Memory->bIsWaiting = true;
				Memory->WaitElapsed = 0.0f;
				Memory->TimeSinceLastLookChange = 0.0f;

				AIController->StopMovement();

				UE_LOG(LogTemp, Verbose, TEXT("[BTTask_PatrolToPoint] Llegó al punto %d, esperando"),
					PMCCharacter->CurrentPatrolIndex);
			}
		}
		return;
	}

	// ---- FASE 2: Esperando en el punto (con rotación opcional) ----

	const FPatrolPoint& CurrentPoint = PMCCharacter->PatrolPoints[PMCCharacter->CurrentPatrolIndex];

	Memory->WaitElapsed += DeltaSeconds;

	// Gestionar rotación aleatoria si el punto tiene bLookAround
	if (CurrentPoint.bLookAround)
	{
		Memory->TimeSinceLastLookChange += DeltaSeconds;

		if (Memory->TimeSinceLastLookChange >= RandomLookChangeInterval)
		{
			// Elegir un nuevo yaw objetivo aleatorio en el rango ±120°
			float BaseYaw = PMCCharacter->GetActorRotation().Yaw;
			float RandomOffset = FMath::RandRange(-120.0f, 120.0f);
			Memory->TargetLookYaw = BaseYaw + RandomOffset;
			Memory->TimeSinceLastLookChange = 0.0f;
		}

		// Interpolar suavemente hacia el yaw objetivo
		FRotator CurrentRotation = PMCCharacter->GetActorRotation();
		float NewYaw = FMath::FixedTurn(
			CurrentRotation.Yaw,
			Memory->TargetLookYaw,
			LookAroundRotationSpeed * DeltaSeconds
		);
		PMCCharacter->SetActorRotation(FRotator(0.0f, NewYaw, 0.0f));
	}

	// ---- FASE 3: Verificar si terminó el tiempo de espera ----

	if (Memory->WaitElapsed >= CurrentPoint.WaitTime)
	{
		UBlackboardComponent* BBComp = OwnerComp.GetBlackboardComponent();
		if (BBComp)
		{
			// Calcular el siguiente índice de patrulla (circular)
			int32 NextIndex = (PMCCharacter->CurrentPatrolIndex + 1) % PMCCharacter->PatrolPoints.Num();

			// Actualizar el índice en el Blackboard y en el personaje
			BBComp->SetValueAsInt(APMCAIController::BB_PatrolIndex, NextIndex);
			PMCCharacter->CurrentPatrolIndex = NextIndex;

			UE_LOG(LogTemp, Verbose, TEXT("[BTTask_PatrolToPoint] Espera completada. Próximo punto: %d"),
				NextIndex);
		}

		FinishLatentTask(OwnerComp, EBTNodeResult::Succeeded);
	}
}

EBTNodeResult::Type UBTTask_PatrolToPoint::AbortTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	// Cancelar movimiento si la tarea es interrumpida
	if (AAIController* AIController = OwnerComp.GetAIOwner())
	{
		AIController->StopMovement();
	}

	return EBTNodeResult::Aborted;
}

uint16 UBTTask_PatrolToPoint::GetInstanceMemorySize() const
{
	return sizeof(FPatrolMemory);
}

FString UBTTask_PatrolToPoint::GetStaticDescription() const
{
	return FString::Printf(
		TEXT("Se mueve al punto de patrulla actual (BB_PatrolIndex).\n"
		"Espera en el punto y avanza al siguiente índice al terminar.\n"
		"Radio de aceptación: %.0f cm."),
		AcceptanceRadius);
}
