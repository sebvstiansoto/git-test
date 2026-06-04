// StaminaComponent.h
// Componente de stamina para el sistema de movimiento de ZonaRoja
// Gestiona el sprint, salto, y penalizaciones a la puntería por cansancio

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "StaminaComponent.generated.h"

// ---------------------------------------------------------
// DELEGADOS
// ---------------------------------------------------------

/** Delegado cuando la stamina llega a cero */
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnStaminaDepleted);

/** Delegado cuando la stamina cambia */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnStaminaChanged,
	float, CurrentStamina, float, MaxStamina);

/**
 * UStaminaComponent
 * Gestiona la stamina del personaje para acciones físicas.
 * El cansancio afecta la puntería, velocidad y capacidad de salto.
 */
UCLASS(ClassGroup = (ZonaRoja), meta = (BlueprintSpawnableComponent))
class ZONAROJA_API UStaminaComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UStaminaComponent();

	// ---------------------------------------------------------
	// CONFIGURACION
	// ---------------------------------------------------------

	/** Stamina máxima del personaje */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Stamina|Configuración")
	float MaxStamina;

	/** Velocidad de regeneración de stamina por segundo (en reposo) */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Stamina|Configuración")
	float StaminaRegenRate;

	/** Velocidad de regeneración de stamina por segundo (caminando) */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Stamina|Configuración")
	float WalkingRegenRate;

	/** Consumo de stamina por segundo durante el sprint */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Stamina|Configuración")
	float SprintStaminaDrain;

	/** Costo de stamina por salto */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Stamina|Configuración")
	float JumpStaminaCost;

	/** Tiempo de delay antes de empezar a regenerar (segundos) */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Stamina|Configuración")
	float RegenDelay;

	/** Stamina mínima para poder iniciar sprint */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Stamina|Configuración")
	float MinStaminaToSprint;

	// ---------------------------------------------------------
	// ESTADO (replicado)
	// ---------------------------------------------------------

	/** Stamina actual del personaje */
	UPROPERTY(ReplicatedUsing = OnRep_CurrentStamina, BlueprintReadOnly, Category = "Stamina|Estado")
	float CurrentStamina;

	/** Si el personaje está actualmente esprintando */
	UPROPERTY(Replicated, BlueprintReadOnly, Category = "Stamina|Estado")
	bool bIsSprinting;

	/** Si la stamina está agotada (penalización activa) */
	UPROPERTY(Replicated, BlueprintReadOnly, Category = "Stamina|Estado")
	bool bIsExhausted;

	// ---------------------------------------------------------
	// FUNCIONES PRINCIPALES
	// ---------------------------------------------------------

	/**
	 * Inicia el consumo de stamina por sprint.
	 * Solo surte efecto si hay stamina suficiente.
	 * @return true si el sprint pudo iniciarse
	 */
	UFUNCTION(BlueprintCallable, Category = "Stamina")
	bool StartSprinting();

	/** Detiene el sprint y comienza la regeneración con delay */
	UFUNCTION(BlueprintCallable, Category = "Stamina")
	void StopSprinting();

	/**
	 * Consume stamina por un salto.
	 * @return true si había stamina suficiente para saltar
	 */
	UFUNCTION(BlueprintCallable, Category = "Stamina")
	bool ConsumeJumpStamina();

	/**
	 * Consume una cantidad arbitraria de stamina.
	 * @param Amount Cantidad de stamina a consumir
	 */
	UFUNCTION(BlueprintCallable, Category = "Stamina")
	void ConsumeStamina(float Amount);

	// ---------------------------------------------------------
	// CONSULTAS
	// ---------------------------------------------------------

	/** Devuelve la stamina como porcentaje (0.0 - 1.0) */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Stamina")
	float GetStaminaPercent() const;

	/** Devuelve si el personaje puede esprintar ahora mismo */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Stamina")
	bool CanSprint() const;

	/** Devuelve si el personaje puede saltar ahora mismo */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Stamina")
	bool CanJump() const;

	/**
	 * Devuelve el multiplicador de dispersión de puntería según el cansancio.
	 * Retorna valores de 1.0 (descansado) a 3.0 (agotado).
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Stamina")
	float GetAimingPenaltyMultiplier() const;

	// ---------------------------------------------------------
	// DELEGADOS
	// ---------------------------------------------------------

	UPROPERTY(BlueprintAssignable, Category = "Stamina|Delegados")
	FOnStaminaDepleted OnStaminaDepleted;

	UPROPERTY(BlueprintAssignable, Category = "Stamina|Delegados")
	FOnStaminaChanged OnStaminaChanged;

	// ---------------------------------------------------------
	// OVERRIDES
	// ---------------------------------------------------------
	virtual void BeginPlay() override;
	virtual void TickComponent(float DeltaTime, ELevelTick TickType,
		FActorComponentTickFunction* ThisTickFunction) override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

protected:
	/** Tiempo desde el último consumo de stamina (para calcular el delay de regen) */
	float TimeSinceLastDrain;

	/** Actualiza la regeneración de stamina en cada frame */
	void UpdateStaminaRegen(float DeltaTime);

	/** Actualiza el consumo de stamina en sprint */
	void UpdateSprintDrain(float DeltaTime);

	/** Callback de replicación de la stamina */
	UFUNCTION()
	void OnRep_CurrentStamina();
};
