// BTTask_PatrolToPoint.h
// Tarea de BehaviorTree para moverse al siguiente punto de patrulla
// Gestiona espera, mirada aleatoria y avance cíclico por los puntos

#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTTaskNode.h"
#include "BTTask_PatrolToPoint.generated.h"

/**
 * UBTTask_PatrolToPoint
 * Tarea del BehaviorTree que controla el movimiento de patrulla del PMC.
 *
 * Flujo de ejecución:
 * 1. Lee BB_PatrolIndex del Blackboard para saber el punto actual
 * 2. Verifica que el PMCCharacter tenga puntos de patrulla definidos
 * 3. Se mueve al punto PatrolPoints[index] mediante pathfinding
 * 4. Al llegar: espera PatrolPoints[index].WaitTime segundos
 * 5. Si bLookAround: rota aleatoriamente durante la espera
 * 6. Incrementa el índice (circular), actualiza BB_PatrolIndex
 * 7. Devuelve Succeeded
 *
 * Usa TickTask para gestionar las fases de movimiento, espera y rotación.
 */
UCLASS()
class ZONAROJA_API UBTTask_PatrolToPoint : public UBTTaskNode
{
	GENERATED_BODY()

public:
	UBTTask_PatrolToPoint();

	/** Radio de aceptación para considerar que llegó al punto de patrulla (cm) */
	UPROPERTY(EditAnywhere, Category = "Patrulla")
	float AcceptanceRadius = 50.0f;

	/** Velocidad de rotación aleatoria durante la espera (grados por segundo) */
	UPROPERTY(EditAnywhere, Category = "Patrulla")
	float LookAroundRotationSpeed = 45.0f;

	/** Intervalo en segundos entre cambios de dirección aleatoria durante la espera */
	UPROPERTY(EditAnywhere, Category = "Patrulla")
	float RandomLookChangeInterval = 1.5f;

	// ============================================================
	// MEMORIA DE LA TAREA
	// ============================================================

	/** Estado interno de la tarea entre frames */
	struct FPatrolMemory
	{
		/** Si el PMC ya llegó al punto y está en fase de espera */
		bool bIsWaiting = false;

		/** Tiempo acumulado en la espera en el punto actual */
		float WaitElapsed = 0.0f;

		/** Tiempo desde el último cambio de dirección de mirada */
		float TimeSinceLastLookChange = 0.0f;

		/** Yaw objetivo actual durante el look-around */
		float TargetLookYaw = 0.0f;

		/** Si el movimiento al punto fue solicitado */
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
