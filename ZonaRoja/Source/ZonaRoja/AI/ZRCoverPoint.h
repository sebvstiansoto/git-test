// ZRCoverPoint.h
// Actor colocable que marca posiciones válidas de cobertura en el nivel
// Usado por el sistema de IA para la búsqueda táctica de cobertura

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "ZRCoverPoint.generated.h"

// Declaraciones anticipadas
class USphereComponent;
class APMCCharacter;

/**
 * AZRCoverPoint
 * Actor colocado por diseñadores en el nivel que marca una posición de cobertura válida.
 * Los PMCs buscan estos actores mediante sphere overlap para encontrar posiciones tácticas.
 * Soporta cobertura alta (de pie) y baja (agachado), con direcciones de asomada configurables.
 */
UCLASS(Blueprintable, BlueprintType, hidecategories = (Rendering, Replication, Input, LOD, Actor, Cooking))
class ZONAROJA_API AZRCoverPoint : public AActor
{
	GENERATED_BODY()

public:
	AZRCoverPoint();

	// ============================================================
	// CONFIGURACION DEL PUNTO DE COBERTURA
	// ============================================================

	/**
	 * Si la cobertura es alta (puede disparar de pie desde aquí).
	 * false = cobertura baja, solo permite disparar agachado.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cobertura")
	bool bIsHighCover = false;

	/**
	 * Dirección mundial para asomarse por el lado izquierdo.
	 * Indica desde qué ángulo el PMC puede disparar asomándose a la izquierda.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cobertura")
	FVector PeekDirectionLeft = FVector(-1.0f, 0.0f, 0.0f);

	/**
	 * Dirección mundial para asomarse por el lado derecho.
	 * Indica desde qué ángulo el PMC puede disparar asomándose a la derecha.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cobertura")
	FVector PeekDirectionRight = FVector(1.0f, 0.0f, 0.0f);

	// ============================================================
	// COMPONENTES
	// ============================================================

	/**
	 * Volumen esférico de 50cm de radio para consultas de overlap en búsquedas de cobertura.
	 * Los PMCs hacen OverlapMultiByChannel para encontrar todos los AZRCoverPoint cercanos.
	 */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Componentes")
	TObjectPtr<USphereComponent> OverlapVolume;

	// ============================================================
	// ESTADO DE OCUPACION
	// ============================================================

	/** Si este punto de cobertura está siendo usado actualmente por un PMC */
	UPROPERTY(BlueprintReadOnly, Category = "Cobertura|Estado")
	bool bIsOccupied = false;

	/** Referencia al PMC que ocupa actualmente esta cobertura */
	UPROPERTY(BlueprintReadOnly, Category = "Cobertura|Estado")
	TObjectPtr<APMCCharacter> OccupyingCharacter;

	// ============================================================
	// FUNCIONES DE OCUPACION
	// ============================================================

	/**
	 * Intenta ocupar este punto de cobertura con el PMC especificado.
	 * Falla si el punto ya está ocupado por otro PMC.
	 * @param Character PMC que quiere ocupar esta cobertura
	 * @return true si la ocupación fue exitosa, false si ya está ocupado
	 */
	UFUNCTION(BlueprintCallable, Category = "Cobertura")
	bool TryOccupy(APMCCharacter* Character);

	/**
	 * Libera este punto de cobertura.
	 * Lo marca como disponible para que otros PMCs puedan usarlo.
	 */
	UFUNCTION(BlueprintCallable, Category = "Cobertura")
	void Release();

	/**
	 * Verifica si esta posición proporciona cobertura real contra una amenaza.
	 * Realiza un sphere trace desde la amenaza hasta esta cobertura.
	 * @param ThreatLocation Posición en el mundo del peligro (jugador)
	 * @return true si esta cobertura bloquea la línea de visión desde ThreatLocation
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Cobertura")
	bool CanProviderCoverFrom(FVector ThreatLocation) const;

	// ============================================================
	// OVERRIDES DE AACTOR
	// ============================================================

	virtual void BeginPlay() override;

#if WITH_EDITOR
	/** Dibuja flechas de dirección de asomada en el editor para facilitar la colocación */
	virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
#endif
};
