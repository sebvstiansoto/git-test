// PMCAIController.h
// Controlador de IA para el personaje PMC
// Gestiona el BehaviorTree, Blackboard y búsqueda de cobertura (estilo EQS)

#pragma once

#include "CoreMinimal.h"
#include "AIController.h"
#include "AI/ZRAITypes.h"
#include "PMCAIController.generated.h"

// Declaraciones anticipadas
class UBehaviorTreeComponent;
class UBlackboardComponent;
class UBehaviorTree;
class APMCCharacter;
class AZRCoverPoint;

/**
 * APMCAIController
 * Controlador de IA que dirige al APMCCharacter mediante un BehaviorTree.
 * Gestiona el Blackboard, el árbol de comportamiento y la búsqueda táctica de cobertura.
 * Las claves del Blackboard se sincronizan con el estado del PMCCharacter.
 */
UCLASS(BlueprintType, Blueprintable)
class ZONAROJA_API APMCAIController : public AAIController
{
	GENERATED_BODY()

public:
	APMCAIController();

	// ============================================================
	// ASSET DEL BEHAVIOR TREE
	// ============================================================

	/** BehaviorTree que controla la lógica de este PMC (asignado en el Blueprint) */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "IA|BehaviorTree")
	TObjectPtr<UBehaviorTree> BehaviorTreeAsset;

	// ============================================================
	// CLAVES DEL BLACKBOARD
	// ============================================================
	// Las constantes FName evitan errores de tipeo y centralizan los nombres de clave

	/** Clave Blackboard: Actor objetivo del PMC (tipo Object) */
	static const FName BB_TargetActor;

	/** Clave Blackboard: Última posición conocida del objetivo (tipo Vector) */
	static const FName BB_LastKnownLocation;

	/** Clave Blackboard: Estado actual del PMC (tipo Enum) */
	static const FName BB_CurrentState;

	/** Clave Blackboard: Índice del punto de patrulla actual (tipo Int) */
	static const FName BB_PatrolIndex;

	/** Clave Blackboard: Si el PMC tiene un objetivo activo (tipo Bool) */
	static const FName BB_HasTarget;

	/** Clave Blackboard: Posición de cobertura seleccionada (tipo Vector) */
	static const FName BB_CoverLocation;

	/** Clave Blackboard: Si el PMC está actualmente en cobertura (tipo Bool) */
	static const FName BB_IsInCover;

	// ============================================================
	// COMPONENTES
	// ============================================================

	/** Componente del árbol de comportamiento */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "IA")
	TObjectPtr<UBehaviorTreeComponent> BTComponent;

	/** Componente del pizarrón de datos compartidos */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "IA")
	TObjectPtr<UBlackboardComponent> BBComponent;

	// ============================================================
	// FUNCIONES DE CONTROL DE COMPORTAMIENTO
	// ============================================================

	/**
	 * Establece el actor objetivo en el Blackboard.
	 * Activa BB_HasTarget y asigna BB_TargetActor.
	 * @param Target Actor que será el objetivo del PMC
	 */
	UFUNCTION(BlueprintCallable, Category = "IA|Blackboard")
	void SetTarget(AActor* Target);

	/**
	 * Limpia el objetivo del Blackboard.
	 * Desactiva BB_HasTarget y borra BB_TargetActor.
	 */
	UFUNCTION(BlueprintCallable, Category = "IA|Blackboard")
	void ClearTarget();

	/**
	 * Sincroniza la clave BB_CurrentState con el estado proporcionado.
	 * @param State Estado del PMC a reflejar en el Blackboard
	 */
	UFUNCTION(BlueprintCallable, Category = "IA|Blackboard")
	void UpdateBlackboardFromState(EPMCState State);

	/**
	 * Actualiza la última posición conocida del objetivo en el Blackboard.
	 * @param Loc Posición mundial a almacenar
	 */
	UFUNCTION(BlueprintCallable, Category = "IA|Blackboard")
	void SetLastKnownLocation(FVector Loc);

	/**
	 * Actualiza las claves de cobertura en el Blackboard.
	 * @param Loc Posición del punto de cobertura seleccionado
	 * @param bInCover Si el PMC está o no en cobertura
	 */
	UFUNCTION(BlueprintCallable, Category = "IA|Blackboard")
	void SetCoverLocation(FVector Loc, bool bInCover);

	/**
	 * Busca el mejor punto de cobertura respecto a una amenaza.
	 * Usa sphere overlap para encontrar actores AZRCoverPoint cercanos
	 * y selecciona el que mejor protege al PMC de la amenaza.
	 * @param ThreatActor Actor que representa la amenaza (jugador)
	 * @return Posición del mejor punto de cobertura encontrado, o ZeroVector si no hay ninguno
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "IA|Táctica")
	FVector FindBestCoverPoint(AActor* ThreatActor) const;

	// ============================================================
	// OVERRIDES DE AAICONTROLLER
	// ============================================================

	virtual void OnPossess(APawn* InPawn) override;
	virtual void OnUnPossess() override;

protected:
	/** Referencia cacheada al PMCCharacter poseído */
	UPROPERTY()
	TObjectPtr<APMCCharacter> PossessedPMC;

	/** Radio de búsqueda de puntos de cobertura en centímetros */
	UPROPERTY(EditDefaultsOnly, Category = "IA|Cobertura")
	float CoverSearchRadius = 2000.0f;
};
