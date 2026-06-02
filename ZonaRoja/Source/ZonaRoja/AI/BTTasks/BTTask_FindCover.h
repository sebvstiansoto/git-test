// BTTask_FindCover.h
// Tarea de BehaviorTree para buscar y moverse a un punto de cobertura
// Utiliza FindBestCoverPoint del AIController para selección táctica

#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTTaskNode.h"
#include "BTTask_FindCover.generated.h"

/**
 * UBTTask_FindCover
 * Tarea del BehaviorTree que busca el mejor punto de cobertura disponible
 * respecto al objetivo actual y ordena al PMC moverse hasta él.
 *
 * Flujo de ejecución:
 * 1. Obtiene el AIController y el PMCCharacter
 * 2. Lee el actor objetivo desde BB_TargetActor
 * 3. Llama a FindBestCoverPoint del controlador
 * 4. Si se encuentra cobertura: actualiza BB_CoverLocation y se mueve
 * 5. Usa TickTask para detectar la llegada mediante PathFollowingComponent
 * 6. Al llegar: marca BB_IsInCover = true y ocupa el AZRCoverPoint más cercano
 * 7. Si no hay cobertura o no hay ruta: devuelve Failed
 */
UCLASS()
class ZONAROJA_API UBTTask_FindCover : public UBTTaskNode
{
	GENERATED_BODY()

public:
	UBTTask_FindCover();

	/** Radio en centímetros dentro del cual aceptar la llegada a la cobertura */
	UPROPERTY(EditAnywhere, Category = "Cobertura")
	float AcceptanceRadius = 50.0f;

	/** Si debe marcar el CoverPoint como ocupado al llegar */
	UPROPERTY(EditAnywhere, Category = "Cobertura")
	bool bOccupyCoverOnArrival = true;

	// ============================================================
	// OVERRIDES DE UBTASKNODE
	// ============================================================

	virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;
	virtual void TickTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds) override;
	virtual EBTNodeResult::Type AbortTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;
	virtual void OnTaskFinished(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory,
		EBTNodeResult::Type TaskResult) override;

	virtual FString GetStaticDescription() const override;
};
