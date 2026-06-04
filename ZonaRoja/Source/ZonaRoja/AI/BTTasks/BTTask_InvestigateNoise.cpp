// BTTask_InvestigateNoise.cpp
// Implementación de la investigación de ruidos con movimiento y rotación post-llegada

#include "AI/BTTasks/BTTask_InvestigateNoise.h"
#include "AI/PMCCharacter.h"
#include "AI/PMCAIController.h"

#include "BehaviorTree/BehaviorTreeComponent.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "AIController.h"
#include "Navigation/PathFollowingComponent.h"

UBTTask_InvestigateNoise::UBTTask_InvestigateNoise()
{
	// Nombre mostrado en el editor del BehaviorTree
	NodeName = TEXT("Investigar Ruido");

	// Habilitar TickTask para gestionar la fase de rotación
	bNotifyTick = true;
	bNotifyTaskFinished = true;
}

EBTNodeResult::Type UBTTask_InvestigateNoise::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	// Inicializar la memoria de la tarea
	FInvestigateMemory* Memory = reinterpret_cast<FInvestigateMemory*>(NodeMemory);
	if (Memory)
	{
		*Memory = FInvestigateMemory(); // Reset al estado inicial
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

	UBlackboardComponent* BBComp = OwnerComp.GetBlackboardComponent();
	if (!BBComp)
	{
		return EBTNodeResult::Failed;
	}

	// Leer la posición de investigación del Blackboard
	FVector InvestigateLocation = BBComp->GetValueAsVector(APMCAIController::BB_LastKnownLocation);

	if (InvestigateLocation.IsNearlyZero())
	{
		UE_LOG(LogTemp, Verbose, TEXT("[BTTask_InvestigateNoise] No hay posición conocida para investigar"));
		return EBTNodeResult::Failed;
	}

	// Solicitar movimiento hacia la posición del ruido
	FAIMoveRequest MoveRequest;
	MoveRequest.SetGoalLocation(InvestigateLocation);
	MoveRequest.SetAcceptanceRadius(InvestigateAcceptanceRadius);
	MoveRequest.SetUsePathfinding(true);
	MoveRequest.SetAllowPartialPath(true); // Permitir llegada parcial si hay obstáculos

	FAIRequestID RequestID = AIController->MoveTo(MoveRequest);

	if (RequestID.IsValid())
	{
		if (Memory)
		{
			Memory->bMoveRequested = true;
		}

		UE_LOG(LogTemp, Verbose, TEXT("[BTTask_InvestigateNoise] Moviéndose a investigar: %s"),
			*InvestigateLocation.ToString());

		return EBTNodeResult::InProgress;
	}

	return EBTNodeResult::Failed;
}

void UBTTask_InvestigateNoise::TickTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds)
{
	FInvestigateMemory* Memory = reinterpret_cast<FInvestigateMemory*>(NodeMemory);
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

	// ---- FASE 1: Verificar si el jugador fue encontrado durante el movimiento ----

	if (PMCCharacter->HasLineOfSightToPlayer())
	{
		// Encontró al jugador durante la investigación: ir a combate
		PMCCharacter->SetState(EPMCState::Combat);
		FinishLatentTask(OwnerComp, EBTNodeResult::Succeeded);
		return;
	}

	// ---- FASE 2: Comprobar si llegó a destino ----

	if (!Memory->bIsLookingAround)
	{
		// Verificar el estado del path following
		UPathFollowingComponent* PathFollowComp = AIController->GetPathFollowingComponent();
		if (PathFollowComp)
		{
			EPathFollowingStatus::Type Status = PathFollowComp->GetStatus();

			if (Status == EPathFollowingStatus::Idle || Status == EPathFollowingStatus::Waiting)
			{
				// Llegó al destino: iniciar fase de mirar alrededor
				Memory->bIsLookingAround = true;
				Memory->LookAroundElapsed = 0.0f;
				Memory->InitialYaw = PMCCharacter->GetActorRotation().Yaw;

				// Detener el movimiento explícitamente
				AIController->StopMovement();

				UE_LOG(LogTemp, Verbose, TEXT("[BTTask_InvestigateNoise] Llegó a destino, iniciando inspección visual"));
			}
		}
		return;
	}

	// ---- FASE 3: Mirar alrededor (rotación ±90°) ----

	Memory->LookAroundElapsed += DeltaSeconds;

	// Calcular la rotación de yaw usando una curva sinusoidal
	// Va de +LookAroundAngle a -LookAroundAngle suavemente durante LookAroundDuration
	float Progress = Memory->LookAroundElapsed / LookAroundDuration; // 0.0 a 1.0
	float YawOffset = FMath::Sin(Progress * PI * 2.0f) * LookAroundAngle;

	float NewYaw = Memory->InitialYaw + YawOffset;
	PMCCharacter->SetActorRotation(FRotator(0.0f, NewYaw, 0.0f));

	// ---- FASE 4: Terminar al agotar el tiempo de inspección ----

	if (Memory->LookAroundElapsed >= LookAroundDuration)
	{
		// Verificar una última vez si el jugador fue encontrado
		if (PMCCharacter->HasLineOfSightToPlayer())
		{
			PMCCharacter->SetState(EPMCState::Combat);
		}
		else
		{
			// No encontró nada: volver a patrulla
			UE_LOG(LogTemp, Verbose, TEXT("[BTTask_InvestigateNoise] Investigación completada, volviendo a patrulla"));
			PMCCharacter->SetState(EPMCState::Patrol);
		}

		// Restaurar rotación inicial
		PMCCharacter->SetActorRotation(FRotator(0.0f, Memory->InitialYaw, 0.0f));

		FinishLatentTask(OwnerComp, EBTNodeResult::Succeeded);
	}
}

EBTNodeResult::Type UBTTask_InvestigateNoise::AbortTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	// Cancelar movimiento si la tarea es interrumpida
	if (AAIController* AIController = OwnerComp.GetAIOwner())
	{
		AIController->StopMovement();
	}

	return EBTNodeResult::Aborted;
}

uint16 UBTTask_InvestigateNoise::GetInstanceMemorySize() const
{
	return sizeof(FInvestigateMemory);
}

FString UBTTask_InvestigateNoise::GetStaticDescription() const
{
	return FString::Printf(
		TEXT("Se mueve a la última posición conocida y mira alrededor %.1fs (±%.0f°).\n"
		"Si detecta al jugador → Combate. Si no → Patrulla."),
		LookAroundDuration, LookAroundAngle);
}
