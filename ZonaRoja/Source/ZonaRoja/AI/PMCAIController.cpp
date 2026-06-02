// PMCAIController.cpp
// Implementación del controlador de IA para PMC
// Gestiona inicio del BehaviorTree, sincronización de Blackboard y búsqueda de cobertura

#include "AI/PMCAIController.h"
#include "AI/PMCCharacter.h"
#include "AI/ZRCoverPoint.h"

#include "BehaviorTree/BehaviorTree.h"
#include "BehaviorTree/BehaviorTreeComponent.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "BehaviorTree/Blackboard/BlackboardKeyType_Bool.h"
#include "BehaviorTree/Blackboard/BlackboardKeyType_Enum.h"
#include "BehaviorTree/Blackboard/BlackboardKeyType_Int.h"
#include "BehaviorTree/Blackboard/BlackboardKeyType_Object.h"
#include "BehaviorTree/Blackboard/BlackboardKeyType_Vector.h"

#include "Kismet/GameplayStatics.h"
#include "EngineUtils.h"

// ------------------------------------------------------------
// DEFINICION DE CLAVES DEL BLACKBOARD
// ------------------------------------------------------------
// Centraliza los nombres para evitar errores de tipeo en todas las tareas

const FName APMCAIController::BB_TargetActor        = TEXT("TargetActor");
const FName APMCAIController::BB_LastKnownLocation  = TEXT("LastKnownLocation");
const FName APMCAIController::BB_CurrentState       = TEXT("CurrentState");
const FName APMCAIController::BB_PatrolIndex        = TEXT("PatrolIndex");
const FName APMCAIController::BB_HasTarget          = TEXT("HasTarget");
const FName APMCAIController::BB_CoverLocation      = TEXT("CoverLocation");
const FName APMCAIController::BB_IsInCover          = TEXT("IsInCover");

APMCAIController::APMCAIController()
{
	// Crear componentes del BehaviorTree y Blackboard
	BTComponent  = CreateDefaultSubobject<UBehaviorTreeComponent>(TEXT("BTComponent"));
	BBComponent  = CreateDefaultSubobject<UBlackboardComponent>(TEXT("BBComponent"));
}

// ------------------------------------------------------------
// CONTROL DE POSESION
// ------------------------------------------------------------

void APMCAIController::OnPossess(APawn* InPawn)
{
	Super::OnPossess(InPawn);

	// Cachear referencia al PMCCharacter
	PossessedPMC = Cast<APMCCharacter>(InPawn);
	if (!PossessedPMC)
	{
		UE_LOG(LogTemp, Warning, TEXT("[PMCAIController] %s poseído un peón que no es APMCCharacter"),
			*GetName());
		return;
	}

	// Inicializar el Blackboard y arrancar el BehaviorTree
	if (BehaviorTreeAsset)
	{
		// UseBlackboard inicializa el BBComponent con el asset del BT
		if (UseBlackboard(BehaviorTreeAsset->BlackboardAsset, BBComponent))
		{
			// Establecer valores iniciales del Blackboard
			BBComponent->SetValueAsBool(BB_HasTarget, false);
			BBComponent->SetValueAsBool(BB_IsInCover, false);
			BBComponent->SetValueAsInt(BB_PatrolIndex, 0);
			BBComponent->SetValueAsEnum(BB_CurrentState,
				static_cast<uint8>(EPMCState::Patrol));

			// Arrancar el árbol de comportamiento
			RunBehaviorTree(BehaviorTreeAsset);

			UE_LOG(LogTemp, Log, TEXT("[PMCAIController] BehaviorTree iniciado para %s"),
				*PossessedPMC->GetName());
		}
		else
		{
			UE_LOG(LogTemp, Warning, TEXT("[PMCAIController] No se pudo inicializar el Blackboard para %s"),
				*GetName());
		}
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("[PMCAIController] No hay BehaviorTreeAsset asignado en %s"),
			*GetName());
	}
}

void APMCAIController::OnUnPossess()
{
	// Detener el BehaviorTree al desposeer
	if (BTComponent)
	{
		BTComponent->StopTree(EBTStopMode::Safe);
	}

	PossessedPMC = nullptr;

	Super::OnUnPossess();
}

// ------------------------------------------------------------
// SINCRONIZACION DEL BLACKBOARD
// ------------------------------------------------------------

void APMCAIController::SetTarget(AActor* Target)
{
	if (!BBComponent)
	{
		return;
	}

	if (Target)
	{
		// Establecer el objetivo y marcar que hay uno activo
		BBComponent->SetValueAsObject(BB_TargetActor, Target);
		BBComponent->SetValueAsBool(BB_HasTarget, true);

		// Actualizar también la última posición conocida
		SetLastKnownLocation(Target->GetActorLocation());
	}
	else
	{
		ClearTarget();
	}
}

void APMCAIController::ClearTarget()
{
	if (!BBComponent)
	{
		return;
	}

	// Limpiar el objetivo del Blackboard
	BBComponent->ClearValue(BB_TargetActor);
	BBComponent->SetValueAsBool(BB_HasTarget, false);
}

void APMCAIController::UpdateBlackboardFromState(EPMCState State)
{
	if (!BBComponent)
	{
		return;
	}

	// Sincronizar el estado del PMC con el Blackboard
	BBComponent->SetValueAsEnum(BB_CurrentState, static_cast<uint8>(State));

	// Si el PMC muere, limpiar el objetivo
	if (State == EPMCState::Dead)
	{
		ClearTarget();
		BBComponent->SetValueAsBool(BB_IsInCover, false);
	}
}

void APMCAIController::SetLastKnownLocation(FVector Loc)
{
	if (!BBComponent)
	{
		return;
	}

	BBComponent->SetValueAsVector(BB_LastKnownLocation, Loc);
}

void APMCAIController::SetCoverLocation(FVector Loc, bool bInCover)
{
	if (!BBComponent)
	{
		return;
	}

	BBComponent->SetValueAsVector(BB_CoverLocation, Loc);
	BBComponent->SetValueAsBool(BB_IsInCover, bInCover);
}

// ------------------------------------------------------------
// BUSQUEDA DE COBERTURA (ESTILO EQS)
// ------------------------------------------------------------

FVector APMCAIController::FindBestCoverPoint(AActor* ThreatActor) const
{
	if (!ThreatActor || !PossessedPMC)
	{
		return FVector::ZeroVector;
	}

	// Recopilar todos los actores de cobertura en el radio de búsqueda
	TArray<FOverlapResult> OverlapResults;
	FCollisionShape SphereShape = FCollisionShape::MakeSphere(CoverSearchRadius);
	FCollisionQueryParams QueryParams(FName("CoverSearch"), false, PossessedPMC);

	bool bFoundOverlaps = GetWorld()->OverlapMultiByObjectType(
		OverlapResults,
		PossessedPMC->GetActorLocation(),
		FQuat::Identity,
		FCollisionObjectQueryParams(ECollisionChannel::ECC_WorldDynamic),
		SphereShape,
		QueryParams
	);

	AZRCoverPoint* BestCoverPoint = nullptr;
	float BestScore = -FLT_MAX;

	FVector ThreatLocation = ThreatActor->GetActorLocation();
	FVector MyLocation = PossessedPMC->GetActorLocation();

	for (const FOverlapResult& Result : OverlapResults)
	{
		AZRCoverPoint* CoverPoint = Cast<AZRCoverPoint>(Result.GetActor());
		if (!CoverPoint)
		{
			continue;
		}

		// Ignorar coberturas ocupadas
		if (CoverPoint->bIsOccupied)
		{
			continue;
		}

		// Verificar que esta cobertura realmente protege de la amenaza
		if (!CoverPoint->CanProviderCoverFrom(ThreatLocation))
		{
			continue;
		}

		// Calcular puntuación de esta cobertura:
		// Mayor puntuación = más cerca del PMC y más lejos de la amenaza
		float DistanceToMe = FVector::Dist(MyLocation, CoverPoint->GetActorLocation());
		float DistanceToThreat = FVector::Dist(ThreatLocation, CoverPoint->GetActorLocation());

		// Normalizar distancias (rango 0-1)
		float NormalizedDistToMe = FMath::Clamp(1.0f - (DistanceToMe / CoverSearchRadius), 0.0f, 1.0f);
		float NormalizedDistToThreat = FMath::Clamp(DistanceToThreat / CoverSearchRadius, 0.0f, 1.0f);

		// Peso: 60% cercanía al PMC + 40% distancia a la amenaza
		float Score = (NormalizedDistToMe * 0.6f) + (NormalizedDistToThreat * 0.4f);

		// Bonus por cobertura alta (puede disparar desde posición de pie)
		if (CoverPoint->bIsHighCover)
		{
			Score += 0.1f;
		}

		if (Score > BestScore)
		{
			BestScore = Score;
			BestCoverPoint = CoverPoint;
		}
	}

	if (BestCoverPoint)
	{
		UE_LOG(LogTemp, Log, TEXT("[PMCAIController] Mejor cobertura encontrada: %s (puntuación: %.2f)"),
			*BestCoverPoint->GetName(), BestScore);
		return BestCoverPoint->GetActorLocation();
	}

	// No se encontró ninguna cobertura válida
	UE_LOG(LogTemp, Verbose, TEXT("[PMCAIController] %s: No se encontró cobertura válida cerca"),
		*GetName());
	return FVector::ZeroVector;
}
