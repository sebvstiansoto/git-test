// BTDecorator_CanSeeTarget.h
// Decorador de BehaviorTree que verifica si el PMC tiene línea de visión al jugador
// Condiciona ramas del árbol a la disponibilidad de LOS

#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTDecorator.h"
#include "BTDecorator_CanSeeTarget.generated.h"

/**
 * UBTDecorator_CanSeeTarget
 * Decorador del BehaviorTree que evalúa si el PMC tiene línea de visión directa al jugador.
 * Utiliza APMCCharacter::HasLineOfSightToPlayer() que realiza un trace personalizado
 * ignorando a los compañeros PMCs.
 *
 * Uso típico: condiciona la rama de "disparar al objetivo" para que solo
 * se ejecute cuando hay LOS real, no solo cuando hay un objetivo en el Blackboard.
 */
UCLASS()
class ZONAROJA_API UBTDecorator_CanSeeTarget : public UBTDecorator
{
	GENERATED_BODY()

public:
	UBTDecorator_CanSeeTarget();

	// ============================================================
	// OVERRIDES DE UBTDECORATOR
	// ============================================================

	/**
	 * Evalúa la condición: ¿tiene el PMC línea de visión al jugador?
	 * @param OwnerComp Componente del BehaviorTree
	 * @param NodeMemory Memoria de nodo (no usada por este decorador)
	 * @return true si HasLineOfSightToPlayer() devuelve true
	 */
	virtual bool CalculateRawConditionValue(UBehaviorTreeComponent& OwnerComp,
		uint8* NodeMemory) const override;

	virtual FString GetStaticDescription() const override;
};
