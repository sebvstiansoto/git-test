// BTTask_FireAtTarget.h
// Tarea de BehaviorTree para disparar al objetivo con dispersión y cadencia configurables
// Realiza hitscan desde el cañón con cono de dispersión según el nivel del PMC

#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTTaskNode.h"
#include "BTTask_FireAtTarget.generated.h"

/**
 * UBTTask_FireAtTarget
 * Tarea del BehaviorTree que ejecuta un disparo hitscan hacia el jugador objetivo.
 * Respeta la cadencia de fuego del PMC y aplica dispersión angular según su precisión.
 *
 * Flujo de ejecución:
 * 1. Verifica línea de visión al jugador
 * 2. Comprueba que ha pasado suficiente tiempo desde el último disparo (FireRate)
 * 3. Calcula dirección de disparo con dispersión aleatoria en cono (VRandCone)
 * 4. Realiza LineTrace hitscan desde el cañón
 * 5. Si impacta al jugador: aplica daño mediante UHealthComponent::ApplyDamage
 * 6. Reinicia el contador de tiempo entre disparos
 * 7. Activa animación de disparo mediante multicast
 */
UCLASS()
class ZONAROJA_API UBTTask_FireAtTarget : public UBTTaskNode
{
	GENERATED_BODY()

public:
	UBTTask_FireAtTarget();

	/** Daño base por impacto antes de modificadores de parte del cuerpo */
	UPROPERTY(EditAnywhere, Category = "Disparo")
	float BaseDamagePerShot = 25.0f;

	/** Distancia máxima del hitscan en centímetros (4km por defecto) */
	UPROPERTY(EditAnywhere, Category = "Disparo")
	float MaxTraceDistance = 400000.0f;

	// ============================================================
	// OVERRIDES DE UBTASKNODE
	// ============================================================

	virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;
	virtual FString GetStaticDescription() const override;

private:
	/**
	 * Mapea el hueso impactado con el resultado del hit a la parte del cuerpo correspondiente.
	 * @param BoneName Nombre del hueso impactado en el trace
	 * @return Parte del cuerpo correspondiente al hueso
	 */
	EBodyPart GetBodyPartFromBone(const FName& BoneName) const;
};
