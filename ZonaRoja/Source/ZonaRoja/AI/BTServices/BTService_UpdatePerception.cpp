// BTService_UpdatePerception.cpp
// Implementación del servicio de actualización de percepción
// Sincroniza el Blackboard con el estado de visión/audición real del PMC

#include "AI/BTServices/BTService_UpdatePerception.h"
#include "AI/PMCCharacter.h"
#include "AI/PMCAIController.h"
#include "Characters/Player/ZRPlayerCharacter.h"

#include "BehaviorTree/BehaviorTreeComponent.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "AIController.h"

UBTService_UpdatePerception::UBTService_UpdatePerception()
{
	// Nombre mostrado en el editor del BehaviorTree
	NodeName = TEXT("Actualizar Percepción");

	// Intervalo de ejecución: cada 0.2 segundos (5 veces por segundo)
	Interval = 0.2f;
	RandomDeviation = 0.05f; // Pequeña variación para evitar spike de CPU sincronizado
}

void UBTService_UpdatePerception::OnBecomeRelevant(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	Super::OnBecomeRelevant(OwnerComp, NodeMemory);

	// Inicializar memoria al activarse el servicio
	FPerceptionServiceMemory* Memory = reinterpret_cast<FPerceptionServiceMemory*>(NodeMemory);
	if (Memory)
	{
		*Memory = FPerceptionServiceMemory();
	}
}

void UBTService_UpdatePerception::TickNode(
	UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds)
{
	FPerceptionServiceMemory* Memory = reinterpret_cast<FPerceptionServiceMemory*>(NodeMemory);
	if (!Memory)
	{
		return;
	}

	APMCAIController* AIController = Cast<APMCAIController>(OwnerComp.GetAIOwner());
	if (!AIController)
	{
		return;
	}

	APMCCharacter* PMCCharacter = Cast<APMCCharacter>(AIController->GetPawn());
	if (!PMCCharacter)
	{
		return;
	}

	UBlackboardComponent* BBComp = OwnerComp.GetBlackboardComponent();
	if (!BBComp)
	{
		return;
	}

	// PMC muerto: no procesar percepción
	if (PMCCharacter->CurrentState == EPMCState::Dead)
	{
		return;
	}

	// ---- Actualizar BB_CurrentState ----
	// Siempre sincronizar el estado del PMC al Blackboard
	BBComp->SetValueAsEnum(APMCAIController::BB_CurrentState,
		static_cast<uint8>(PMCCharacter->CurrentState));

	// ---- Verificar línea de visión al jugador ----
	bool bHasLOS = PMCCharacter->HasLineOfSightToPlayer();

	if (bHasLOS)
	{
		// Tiene LOS: actualizar posición conocida y reiniciar contador
		Memory->TimeSinceLastSight = 0.0f;

		// Obtener el jugador más cercano para actualizar su posición
		AZRPlayerCharacter* NearestPlayer = PMCCharacter->GetNearestPlayer();
		if (NearestPlayer)
		{
			// Actualizar la última posición conocida en el Blackboard y en el personaje
			FVector PlayerLocation = NearestPlayer->GetActorLocation();
			PMCCharacter->LastKnownPlayerLocation = PlayerLocation;
			AIController->SetLastKnownLocation(PlayerLocation);

			// Confirmar que hay objetivo activo
			BBComp->SetValueAsBool(APMCAIController::BB_HasTarget, true);

			// Si el objetivo no está aún en el Blackboard, establecerlo
			if (!BBComp->GetValueAsObject(APMCAIController::BB_TargetActor))
			{
				AIController->SetTarget(NearestPlayer);
			}
		}
	}
	else
	{
		// Sin LOS: acumular tiempo sin visión
		Memory->TimeSinceLastSight += DeltaSeconds;

		// Verificar si tiene objetivo en el Blackboard (pero sin LOS actual)
		bool bHasTarget = BBComp->GetValueAsBool(APMCAIController::BB_HasTarget);

		if (bHasTarget && Memory->TimeSinceLastSight >= LostSightTimeoutSeconds)
		{
			// Perdió visión del jugador por demasiado tiempo: transicionar a búsqueda
			if (PMCCharacter->CurrentState == EPMCState::Combat)
			{
				UE_LOG(LogTemp, Log, TEXT("[BTService_UpdatePerception] %s perdió LOS por %.1fs, → Alert"),
					*PMCCharacter->GetName(), Memory->TimeSinceLastSight);

				PMCCharacter->SetState(EPMCState::Alert);

				// Limpiar el objetivo activo (sigue habiendo última posición conocida)
				AIController->ClearTarget();
			}
		}
	}
}

uint16 UBTService_UpdatePerception::GetInstanceMemorySize() const
{
	return sizeof(FPerceptionServiceMemory);
}

FString UBTService_UpdatePerception::GetStaticDescription() const
{
	return FString::Printf(
		TEXT("Actualiza el Blackboard con el estado de percepción.\n"
		"Intervalo: %.1fs. Timeout sin LOS: %.1fs antes de → Alert."),
		Interval, LostSightTimeoutSeconds);
}
