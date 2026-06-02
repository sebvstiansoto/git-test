// BTDecorator_IsInCombat.h
// Decorador de BehaviorTree que verifica si el PMC está en estado de combate activo
// Condiciona las ramas de combate del árbol de comportamiento

#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTDecorator.h"
#include "BTDecorator_IsInCombat.generated.h"

/**
 * UBTDecorator_IsInCombat
 * Decorador del BehaviorTree que evalúa si el PMC se encuentra en estado de combate.
 * Comprueba directamente CurrentState == EPMCState::Combat en el APMCCharacter.
 *
 * Uso típico: raíz de la sub-rama de comportamiento de combate (disparar, buscar cobertura,
 * flanquear) que solo debe ejecutarse cuando el PMC está activamente en combate.
 */
UCLASS()
class ZONAROJA_API UBTDecorator_IsInCombat : public UBTDecorator
{
	GENERATED_BODY()

public:
	UBTDecorator_IsInCombat();

	// ============================================================
	// OVERRIDES DE UBTDECORATOR
	// ============================================================

	/**
	 * Evalúa la condición: ¿está el PMC en estado de combate?
	 * @param OwnerComp Componente del BehaviorTree
	 * @param NodeMemory Memoria de nodo (no usada por este decorador)
	 * @return true si CurrentState == EPMCState::Combat
	 */
	virtual bool CalculateRawConditionValue(UBehaviorTreeComponent& OwnerComp,
		uint8* NodeMemory) const override;

	virtual FString GetStaticDescription() const override;
};
