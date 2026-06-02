// BTService_ChooseCoverAction.h
// Servicio de BehaviorTree que elige aleatoriamente la acción táctica del PMC en cobertura
// Se ejecuta cada 0.5s cuando el PMC está en cobertura

#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTService.h"
#include "AI/ZRAITypes.h"
#include "BTService_ChooseCoverAction.generated.h"

/**
 * UBTService_ChooseCoverAction
 * Servicio del BehaviorTree que determina el comportamiento táctico del PMC mientras está en cobertura.
 * Se ejecuta cada 0.5 segundos para tomar decisiones de combate desde cobertura.
 *
 * Distribución de probabilidad de acciones:
 * - 60% → Permanecer en cobertura (ECoverAction::TakingCover)
 * - 25% → Asomarse y disparar (ECoverAction::PeekingLeft o PeekingRight aleatoriamente)
 * - 15% → Avanzar a nueva cobertura (ECoverAction::None + disparar FindCover)
 *
 * Si el PMC no está en cobertura (BB_IsInCover = false), el servicio no hace nada.
 */
UCLASS()
class ZONAROJA_API UBTService_ChooseCoverAction : public UBTService
{
	GENERATED_BODY()

public:
	UBTService_ChooseCoverAction();

	/** Probabilidad (0-1) de permanecer en cobertura sin hacer nada */
	UPROPERTY(EditAnywhere, Category = "Cobertura|Probabilidades",
		meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float StayInCoverChance = 0.60f;

	/** Probabilidad (0-1) de asomarse para disparar */
	UPROPERTY(EditAnywhere, Category = "Cobertura|Probabilidades",
		meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float PeekAndFireChance = 0.25f;

	/**
	 * Probabilidad de avanzar a nueva cobertura.
	 * Se calcula automáticamente como (1 - StayInCoverChance - PeekAndFireChance).
	 * Mostrada solo como referencia visual en el editor.
	 */
	UPROPERTY(VisibleAnywhere, Category = "Cobertura|Probabilidades")
	float AdvanceCoverChance = 0.15f;

	// ============================================================
	// OVERRIDES DE UBTSERVICE
	// ============================================================

	/**
	 * Se llama cada 0.5 segundos mientras el nodo padre está activo.
	 * Elige y aplica la acción de cobertura si el PMC está en cobertura.
	 */
	virtual void TickNode(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory,
		float DeltaSeconds) override;

	virtual FString GetStaticDescription() const override;

#if WITH_EDITOR
	/** Actualiza AdvanceCoverChance cuando cambian los otros valores en el editor */
	virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
#endif
};
