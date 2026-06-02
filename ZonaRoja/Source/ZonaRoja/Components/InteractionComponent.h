// InteractionComponent.h
// Componente de interacción con objetos del mundo en ZonaRoja
// Detecta objetos interactuables mediante raycasting periódico

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "InteractionComponent.generated.h"

// Declaraciones anticipadas
class UCameraComponent;

// ---------------------------------------------------------
// INTERFAZ DE OBJETOS INTERACTUABLES
// ---------------------------------------------------------

/** Delegado para cuando el jugador mira hacia un objeto interactuable */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnInteractableFound,
	AActor*, InteractableActor, const FText&, InteractionPrompt);

/** Delegado para cuando el jugador deja de mirar al objeto */
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnInteractableLost);

/**
 * UInteractionComponent
 * Componente que realiza raycasting hacia objetos interactuables.
 * Actualiza el objetivo de interacción mediante un timer cada 0.1 segundos.
 * Notifica a la UI del objeto bajo el cursor para mostrar prompts de interacción.
 */
UCLASS(ClassGroup = (ZonaRoja), meta = (BlueprintSpawnableComponent))
class ZONAROJA_API UInteractionComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UInteractionComponent();

	// ---------------------------------------------------------
	// CONFIGURACION
	// ---------------------------------------------------------

	/** Distancia máxima del raycast de interacción en centímetros */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Interacción|Configuración")
	float InteractionRange;

	/** Radio de la traza de esfera para detectar objetos (0 = línea recta) */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Interacción|Configuración")
	float TraceRadius;

	/** Frecuencia de actualización del raycast en segundos */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Interacción|Configuración")
	float TraceTickRate;

	/** Canal de colisión para la detección de interacción */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Interacción|Configuración")
	TEnumAsByte<ECollisionChannel> InteractionChannel;

	// ---------------------------------------------------------
	// ESTADO ACTUAL
	// ---------------------------------------------------------

	/** Actor actualmente bajo el cursor de interacción */
	UPROPERTY(BlueprintReadOnly, Category = "Interacción|Estado")
	TObjectPtr<AActor> CurrentInteractableActor;

	/** Prompt de texto del objeto actual (p.ej. "Recoger AK-103") */
	UPROPERTY(BlueprintReadOnly, Category = "Interacción|Estado")
	FText CurrentInteractionPrompt;

	// ---------------------------------------------------------
	// FUNCIONES PRINCIPALES
	// ---------------------------------------------------------

	/**
	 * Activa el componente de interacción e inicia el timer.
	 * Debe llamarse cuando el personaje es poseído.
	 */
	UFUNCTION(BlueprintCallable, Category = "Interacción")
	void EnableInteraction();

	/**
	 * Desactiva el componente de interacción y limpia el timer.
	 */
	UFUNCTION(BlueprintCallable, Category = "Interacción")
	void DisableInteraction();

	/**
	 * Obtiene el actor interactuable actualmente apuntado.
	 * @return Actor interactuable o nullptr si no hay ninguno
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Interacción")
	AActor* GetCurrentInteractableActor() const { return CurrentInteractableActor; }

	/**
	 * Fuerza una actualización del raycast inmediata.
	 */
	UFUNCTION(BlueprintCallable, Category = "Interacción")
	void ForceUpdateTrace();

	// ---------------------------------------------------------
	// DELEGADOS
	// ---------------------------------------------------------

	/** Notifica cuando un objeto interactuable está en rango */
	UPROPERTY(BlueprintAssignable, Category = "Interacción|Delegados")
	FOnInteractableFound OnInteractableFound;

	/** Notifica cuando ya no hay objeto interactuable en rango */
	UPROPERTY(BlueprintAssignable, Category = "Interacción|Delegados")
	FOnInteractableLost OnInteractableLost;

	// ---------------------------------------------------------
	// OVERRIDES
	// ---------------------------------------------------------
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

protected:
	/** Ejecuta el raycast de interacción */
	void PerformInteractionTrace();

	/**
	 * Construye el punto de inicio y dirección de la traza
	 * usando la cámara del personaje.
	 * @param OutStart Posición de inicio de la traza
	 * @param OutEnd Posición final de la traza
	 * @return true si se obtuvo correctamente la cámara
	 */
	bool GetTraceStartAndEnd(FVector& OutStart, FVector& OutEnd) const;

	/** Timer handle para el tick periódico de la traza */
	FTimerHandle InteractionTraceTimer;

	/** Cámara del personaje poseedor (se obtiene en BeginPlay) */
	TWeakObjectPtr<UCameraComponent> OwnerCamera;
};
