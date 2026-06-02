// BTTask_InvestigateNoise.h
// Tarea de BehaviorTree para investigar un ruido escuchado
// El PMC se mueve a la posición y mira alrededor antes de volver a patrullar

#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTTaskNode.h"
#include "BTTask_InvestigateNoise.generated.h"

/**
 * UBTTask_InvestigateNoise
 * Tarea del BehaviorTree que envía al PMC a investigar el origen de un ruido.
 *
 * Flujo de ejecución:
 * 1. Lee BB_LastKnownLocation del Blackboard
 * 2. Se mueve a esa posición mediante pathfinding
 * 3. Al llegar: inicia fase de "mirar alrededor" (rotación ±90° en 2 segundos)
 * 4. Si durante la investigación detecta al jugador → SetState(Combat) → Succeeded
 * 5. Si termina sin detectar nada → SetState(Patrol) → Succeeded
 *
 * Usa TickTask para gestionar la fase de rotación post-llegada.
 */
UCLASS()
class ZONAROJA_API UBTTask_InvestigateNoise : public UBTTaskNode
{
	GENERATED_BODY()

public:
	UBTTask_InvestigateNoise();

	/** Tiempo total en segundos para la fase de mirar alrededor */
	UPROPERTY(EditAnywhere, Category = "Investigación")
	float LookAroundDuration = 2.0f;

	/** Ángulo máximo de rotación a cada lado durante la investigación (grados) */
	UPROPERTY(EditAnywhere, Category = "Investigación")
	float LookAroundAngle = 90.0f;

	/** Radio de aceptación para llegar a la posición de investigación (cm) */
	UPROPERTY(EditAnywhere, Category = "Investigación")
	float InvestigateAcceptanceRadius = 100.0f;

	// ============================================================
	// MEMORIA DE LA TAREA (estado entre frames de TickTask)
	// ============================================================

	/** Estructura que almacena el estado de esta tarea entre ticks */
	struct FInvestigateMemory
	{
		/** Si el PMC ya llegó a la posición y está en fase de rotación */
		bool bIsLookingAround = false;

		/** Tiempo acumulado en la fase de rotación (segundos) */
		float LookAroundElapsed = 0.0f;

		/** Rotación inicial del yaw al empezar a mirar alrededor */
		float InitialYaw = 0.0f;

		/** Si el movimiento de llegada ya fue solicitado */
		bool bMoveRequested = false;
	};

	// ============================================================
	// OVERRIDES DE UBTASKNODE
	// ============================================================

	virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;
	virtual void TickTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds) override;
	virtual EBTNodeResult::Type AbortTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;

	virtual uint16 GetInstanceMemorySize() const override;
	virtual FString GetStaticDescription() const override;
};
