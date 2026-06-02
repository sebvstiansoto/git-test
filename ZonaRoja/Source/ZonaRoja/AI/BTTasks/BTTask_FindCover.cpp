// BTTask_FindCover.cpp
// Implementación de la tarea de búsqueda y movimiento a cobertura

#include "AI/BTTasks/BTTask_FindCover.h"
#include "AI/PMCCharacter.h"
#include "AI/PMCAIController.h"
#include "AI/ZRCoverPoint.h"

#include "BehaviorTree/BehaviorTreeComponent.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "AIController.h"
#include "NavigationSystem.h"
#include "Navigation/PathFollowingComponent.h"
#include "Kismet/GameplayStatics.h"

UBTTask_FindCover::UBTTask_FindCover()
{
	// Nombre mostrado en el editor del BehaviorTree
	NodeName = TEXT("Buscar Cobertura");

	// Esta tarea puede tardar múltiples frames (movimiento asíncrono)
	bNotifyTick = true;
	bNotifyTaskFinished = true;
}

EBTNodeResult::Type UBTTask_FindCover::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	// Obtener el AIController
	APMCAIController* AIController = Cast<APMCAIController>(OwnerComp.GetAIOwner());
	if (!AIController)
	{
		UE_LOG(LogTemp, Warning, TEXT("[BTTask_FindCover] No se encontró APMCAIController"));
		return EBTNodeResult::Failed;
	}

	// Obtener el PMCCharacter
	APMCCharacter* PMCCharacter = Cast<APMCCharacter>(AIController->GetPawn());
	if (!PMCCharacter)
	{
		UE_LOG(LogTemp, Warning, TEXT("[BTTask_FindCover] No se encontró APMCCharacter"));
		return EBTNodeResult::Failed;
	}

	// Obtener el Blackboard
	UBlackboardComponent* BBComp = OwnerComp.GetBlackboardComponent();
	if (!BBComp)
	{
		return EBTNodeResult::Failed;
	}

	// Leer el actor objetivo del Blackboard
	AActor* ThreatActor = Cast<AActor>(BBComp->GetValueAsObject(APMCAIController::BB_TargetActor));
	if (!ThreatActor)
	{
		UE_LOG(LogTemp, Verbose, TEXT("[BTTask_FindCover] No hay objetivo en Blackboard"));
		return EBTNodeResult::Failed;
	}

	// Buscar el mejor punto de cobertura respecto al objetivo
	FVector BestCoverLocation = AIController->FindBestCoverPoint(ThreatActor);

	if (BestCoverLocation.IsNearlyZero())
	{
		// No se encontró ninguna cobertura válida
		UE_LOG(LogTemp, Verbose, TEXT("[BTTask_FindCover] No se encontró cobertura válida"));
		return EBTNodeResult::Failed;
	}

	// Actualizar el Blackboard con la posición de cobertura encontrada
	// Se marca como false (aún no llegó a la cobertura)
	AIController->SetCoverLocation(BestCoverLocation, false);

	// Ordenar al PMC moverse hacia la cobertura
	FAIMoveRequest MoveRequest;
	MoveRequest.SetGoalLocation(BestCoverLocation);
	MoveRequest.SetAcceptanceRadius(AcceptanceRadius);
	MoveRequest.SetUsePathfinding(true);
	MoveRequest.SetAllowPartialPath(false);

	FAIRequestID MoveRequestID = AIController->MoveTo(MoveRequest);

	if (MoveRequestID.IsValid())
	{
		// Movimiento iniciado correctamente: la tarea usa TickTask para detectar la llegada
		UE_LOG(LogTemp, Verbose, TEXT("[BTTask_FindCover] Moviéndose a cobertura en %s"),
			*BestCoverLocation.ToString());
		return EBTNodeResult::InProgress;
	}
	else
	{
		// El movimiento no pudo iniciarse (sin ruta al destino)
		UE_LOG(LogTemp, Warning, TEXT("[BTTask_FindCover] No se pudo iniciar movimiento a cobertura"));
		return EBTNodeResult::Failed;
	}
}

void UBTTask_FindCover::TickTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds)
{
	APMCAIController* AIController = Cast<APMCAIController>(OwnerComp.GetAIOwner());
	if (!AIController)
	{
		FinishLatentTask(OwnerComp, EBTNodeResult::Failed);
		return;
	}

	// Comprobar el estado del path following para detectar llegada
	UPathFollowingComponent* PathFollowComp = AIController->GetPathFollowingComponent();
	if (!PathFollowComp)
	{
		FinishLatentTask(OwnerComp, EBTNodeResult::Failed);
		return;
	}

	EPathFollowingStatus::Type Status = PathFollowComp->GetStatus();

	if (Status == EPathFollowingStatus::Idle || Status == EPathFollowingStatus::Waiting)
	{
		// Llegó a la cobertura: marcar como en cobertura en el Blackboard
		UBlackboardComponent* BBComp = OwnerComp.GetBlackboardComponent();
		if (BBComp)
		{
			FVector CoverLocation = BBComp->GetValueAsVector(APMCAIController::BB_CoverLocation);
			AIController->SetCoverLocation(CoverLocation, true); // Ahora está en cobertura
		}

		// Intentar ocupar el AZRCoverPoint más cercano si bOccupyCoverOnArrival está activo
		if (bOccupyCoverOnArrival)
		{
			APMCCharacter* PMCCharacter = Cast<APMCCharacter>(AIController->GetPawn());
			if (PMCCharacter)
			{
				TArray<AActor*> AllCoverPoints;
				UGameplayStatics::GetAllActorsOfClass(
					OwnerComp.GetWorld(), AZRCoverPoint::StaticClass(), AllCoverPoints);

				float MinDist = 200.0f; // Umbral máximo: 2 metros
				AZRCoverPoint* NearestPoint = nullptr;

				for (AActor* CoverActor : AllCoverPoints)
				{
					AZRCoverPoint* Point = Cast<AZRCoverPoint>(CoverActor);
					if (Point && !Point->bIsOccupied)
					{
						float Dist = FVector::Dist(PMCCharacter->GetActorLocation(),
							Point->GetActorLocation());
						if (Dist < MinDist)
						{
							MinDist = Dist;
							NearestPoint = Point;
						}
					}
				}

				if (NearestPoint)
				{
					NearestPoint->TryOccupy(PMCCharacter);
					PMCCharacter->CurrentCoverActor = NearestPoint;
				}
			}
		}

		FinishLatentTask(OwnerComp, EBTNodeResult::Succeeded);
	}
	else if (Status == EPathFollowingStatus::Paused)
	{
		// El path following fue pausado (posiblemente por colisión o bloqueo)
		// Fallar para que el BT busque otra cobertura
		FinishLatentTask(OwnerComp, EBTNodeResult::Failed);
	}
	// Si el estado es Moving, continuar esperando
}

EBTNodeResult::Type UBTTask_FindCover::AbortTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	// Cancelar movimiento si la tarea es abortada
	if (AAIController* AIController = OwnerComp.GetAIOwner())
	{
		AIController->StopMovement();
	}

	return EBTNodeResult::Aborted;
}

void UBTTask_FindCover::OnTaskFinished(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory,
	EBTNodeResult::Type TaskResult)
{
	// Cleanup adicional al terminar la tarea (ya sea éxito o fallo)
	// Si falló, limpiar el estado de cobertura para intentar de nuevo en el próximo ciclo
	if (TaskResult == EBTNodeResult::Failed)
	{
		APMCAIController* AIController = Cast<APMCAIController>(OwnerComp.GetAIOwner());
		if (AIController)
		{
			// No limpiar BB_CoverLocation para que el servicio pueda intentar de nuevo
			// Solo marcar que no está en cobertura
			UBlackboardComponent* BBComp = OwnerComp.GetBlackboardComponent();
			if (BBComp)
			{
				BBComp->SetValueAsBool(APMCAIController::BB_IsInCover, false);
			}
		}
	}
}

FString UBTTask_FindCover::GetStaticDescription() const
{
	return FString::Printf(
		TEXT("Busca el mejor punto de cobertura respecto al objetivo y se mueve hasta él.\n"
		"Falla si no hay cobertura disponible. Radio de aceptación: %.0f cm."),
		AcceptanceRadius);
}
