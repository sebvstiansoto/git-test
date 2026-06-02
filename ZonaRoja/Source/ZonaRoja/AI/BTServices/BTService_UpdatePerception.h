// BTService_UpdatePerception.h
// Servicio de BehaviorTree que actualiza el estado de percepción del PMC
// Se ejecuta cada 0.2s para mantener el Blackboard sincronizado con la percepción real

#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTService.h"
#include "BTService_UpdatePerception.generated.h"

/**
 * UBTService_UpdatePerception
 * Servicio del BehaviorTree que actualiza periódicamente el estado de percepción.
 * Se ejecuta cada 0.2 segundos mientras el nodo padre está activo.
 *
 * Comportamiento:
 * - Si el PMC tiene LOS al jugador: actualiza BB_LastKnownLocation, confirma BB_HasTarget
 * - Si tiene objetivo pero perdió LOS durante más de 5 segundos: transiciona a Alert
 * - Actualiza BB_CurrentState con el estado actual del PMC en cada tick
 *
 * Este servicio es el "puente" entre el mundo real (percepción del personaje)
 * y el Blackboard que dirige el BehaviorTree.
 */
UCLASS()
class ZONAROJA_API UBTService_UpdatePerception : public UBTService
{
	GENERATED_BODY()

public:
	UBTService_UpdatePerception();

	/**
	 * Segundos sin LOS antes de transicionar de Combat a Alert (modo búsqueda).
	 * Si el PMC pierde visión del jugador durante este tiempo, pasa a buscar.
	 */
	UPROPERTY(EditAnywhere, Category = "Percepción")
	float LostSightTimeoutSeconds = 5.0f;

	// ============================================================
	// MEMORIA DEL SERVICIO
	// ============================================================

	/** Estado interno del servicio entre ticks */
	struct FPerceptionServiceMemory
	{
		/** Tiempo acumulado sin línea de visión al jugador (segundos) */
		float TimeSinceLastSight = 0.0f;
	};

	// ============================================================
	// OVERRIDES DE UBTSERVICE
	// ============================================================

	/**
	 * Se llama periódicamente (cada Interval segundos, por defecto 0.2s).
	 * Actualiza el Blackboard con el estado de percepción actual.
	 */
	virtual void TickNode(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory,
		float DeltaSeconds) override;

	virtual void OnBecomeRelevant(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;

	virtual uint16 GetInstanceMemorySize() const override;
	virtual FString GetStaticDescription() const override;
};
