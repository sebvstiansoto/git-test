// BTService_ChooseCoverAction.cpp
// Implementación del servicio de selección aleatoria de acciones tácticas en cobertura

#include "AI/BTServices/BTService_ChooseCoverAction.h"
#include "AI/PMCCharacter.h"
#include "AI/PMCAIController.h"

#include "BehaviorTree/BehaviorTreeComponent.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "AIController.h"

UBTService_ChooseCoverAction::UBTService_ChooseCoverAction()
{
	// Nombre mostrado en el editor del BehaviorTree
	NodeName = TEXT("Elegir Acción de Cobertura");

	// Intervalo de 0.5 segundos: toma decisiones tácticas dos veces por segundo
	Interval = 0.5f;
	RandomDeviation = 0.1f; // Variación para evitar sincronización entre múltiples PMCs
}

void UBTService_ChooseCoverAction::TickNode(
	UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds)
{
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

	// Solo actuar si el PMC está en combate
	if (PMCCharacter->CurrentState != EPMCState::Combat)
	{
		return;
	}

	// Solo actuar si el PMC está en cobertura
	bool bIsInCover = BBComp->GetValueAsBool(APMCAIController::BB_IsInCover);
	if (!bIsInCover)
	{
		// Fuera de cobertura: asegurarse de que la acción sea None
		if (PMCCharacter->CurrentCoverAction != ECoverAction::None &&
			PMCCharacter->CurrentCoverAction != ECoverAction::TakingCover)
		{
			PMCCharacter->CurrentCoverAction = ECoverAction::None;
		}
		return;
	}

	// ---- Selección aleatoria de acción táctica ----

	float RollValue = FMath::FRand(); // Número aleatorio entre 0.0 y 1.0

	ECoverAction ChosenAction = ECoverAction::TakingCover; // Valor por defecto

	if (RollValue < StayInCoverChance)
	{
		// Acción 1: Permanecer en cobertura (60%)
		ChosenAction = ECoverAction::TakingCover;

		UE_LOG(LogTemp, Verbose, TEXT("[BTService_ChooseCoverAction] %s: Permanecer en cobertura (roll=%.2f)"),
			*PMCCharacter->GetName(), RollValue);
	}
	else if (RollValue < StayInCoverChance + PeekAndFireChance)
	{
		// Acción 2: Asomarse para disparar (25%)
		// Elegir aleatoriamente lado izquierdo o derecho
		bool bPeekLeft = FMath::RandBool();
		ChosenAction = bPeekLeft ? ECoverAction::PeekingLeft : ECoverAction::PeekingRight;

		UE_LOG(LogTemp, Verbose, TEXT("[BTService_ChooseCoverAction] %s: Asomarse %s (roll=%.2f)"),
			*PMCCharacter->GetName(),
			bPeekLeft ? TEXT("izquierda") : TEXT("derecha"),
			RollValue);
	}
	else
	{
		// Acción 3: Avanzar a nueva cobertura (15%)
		// Señalizar al PMC que debe buscar nueva cobertura
		ChosenAction = ECoverAction::None;

		// Limpiar el estado de cobertura actual para que el BT busque nueva cobertura
		AIController->SetCoverLocation(FVector::ZeroVector, false);

		// Liberar el CoverPoint actual si está ocupando uno
		if (PMCCharacter->CurrentCoverActor)
		{
			// Intentar castear y liberar el punto de cobertura
			// (el AZRCoverPoint se libera en el BTTask_FindCover al ocupar uno nuevo)
			PMCCharacter->CurrentCoverActor = nullptr;
		}

		UE_LOG(LogTemp, Verbose, TEXT("[BTService_ChooseCoverAction] %s: Avanzar a nueva cobertura (roll=%.2f)"),
			*PMCCharacter->GetName(), RollValue);
	}

	// Aplicar la acción elegida al personaje
	PMCCharacter->CurrentCoverAction = ChosenAction;
}

FString UBTService_ChooseCoverAction::GetStaticDescription() const
{
	return FString::Printf(
		TEXT("Elige acción táctica desde cobertura cada %.1fs:\n"
		"  %.0f%% permanecer | %.0f%% asomarse | %.0f%% avanzar"),
		Interval,
		StayInCoverChance * 100.0f,
		PeekAndFireChance * 100.0f,
		(1.0f - StayInCoverChance - PeekAndFireChance) * 100.0f);
}

#if WITH_EDITOR
void UBTService_ChooseCoverAction::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);

	// Actualizar la probabilidad de avance (complemento de las otras dos)
	float AdvanceChance = 1.0f - StayInCoverChance - PeekAndFireChance;
	AdvanceCoverChance = FMath::Max(0.0f, AdvanceChance);

	// Asegurarse de que las probabilidades no superen 1.0 en total
	if (StayInCoverChance + PeekAndFireChance > 1.0f)
	{
		UE_LOG(LogTemp, Warning,
			TEXT("[BTService_ChooseCoverAction] Las probabilidades suman más de 1.0. Se recortará la probabilidad de asomarse."));
		PeekAndFireChance = FMath::Max(0.0f, 1.0f - StayInCoverChance);
		AdvanceCoverChance = 0.0f;
	}
}
#endif
