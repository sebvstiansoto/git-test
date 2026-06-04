// ZRPlayerCharacter.h
// Personaje del jugador para ZonaRoja
// Integra todos los componentes: salud, inventario, stamina, armas e interacción

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "Data/ZRTypes.h"
#include "ZRPlayerCharacter.generated.h"

// Declaraciones anticipadas de componentes
class UHealthComponent;
class UInventoryComponent;
class UStaminaComponent;
class UWeaponComponent;
class UInteractionComponent;
class USpringArmComponent;
class UCameraComponent;
class UMotionWarpingComponent;
class AWeaponBase;

/**
 * AZRPlayerCharacter
 * Personaje principal del jugador en ZonaRoja.
 * Combina movimiento táctico (sprint, agacharse, prono, lean) con
 * los sistemas de combate, inventario y salud por partes del cuerpo.
 */
UCLASS(Blueprintable)
class ZONAROJA_API AZRPlayerCharacter : public ACharacter
{
	GENERATED_BODY()

public:
	AZRPlayerCharacter();

	// ---------------------------------------------------------
	// COMPONENTES PRINCIPALES
	// ---------------------------------------------------------

	/** Componente de salud con sistema de daño por partes del cuerpo */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Componentes")
	TObjectPtr<UHealthComponent> HealthComponent;

	/** Componente de inventario tipo cuadrícula */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Componentes")
	TObjectPtr<UInventoryComponent> InventoryComponent;

	/** Componente de stamina para sprint y salto */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Componentes")
	TObjectPtr<UStaminaComponent> StaminaComponent;

	/** Componente de gestión y cambio de armas */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Componentes")
	TObjectPtr<UWeaponComponent> WeaponComponent;

	/** Componente de detección de objetos interactuables */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Componentes")
	TObjectPtr<UInteractionComponent> InteractionComponent;

	/** Brazo del resorte para la cámara en tercera persona */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Cámara")
	TObjectPtr<USpringArmComponent> SpringArmComponent;

	/** Cámara del jugador */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Cámara")
	TObjectPtr<UCameraComponent> CameraComponent;

	/** Componente de Motion Warping para animaciones contextuales */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Animaciones")
	TObjectPtr<UMotionWarpingComponent> MotionWarpingComponent;

	/**
	 * Mesh de brazos en primera persona (solo visible para el jugador local).
	 * Muestra las manos y el arma desde la perspectiva FPP.
	 * El cuerpo completo (Mesh heredado de ACharacter) es visible para TODOS
	 * los demás jugadores pero invisible para el dueño.
	 */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Cámara|Primera Persona")
	TObjectPtr<USkeletalMeshComponent> FPPMesh;

	// ---------------------------------------------------------
	// ESTADO DEL MOVIMIENTO (replicado)
	// ---------------------------------------------------------

	/** Si el personaje está esprintando */
	UPROPERTY(Replicated, BlueprintReadOnly, Category = "Movimiento|Estado")
	bool bIsSprinting;

	/** Si el personaje está apuntando (ADS) */
	UPROPERTY(Replicated, BlueprintReadOnly, Category = "Movimiento|Estado")
	bool bIsAiming;

	/** Valor de inclinación lateral (-1 izquierda, 0 neutral, 1 derecha) */
	UPROPERTY(Replicated, BlueprintReadOnly, Category = "Movimiento|Estado")
	float LeanAmount;

	/** Si el personaje está en posición prono */
	UPROPERTY(Replicated, BlueprintReadOnly, Category = "Movimiento|Estado")
	bool bIsProne;

	// ---------------------------------------------------------
	// CONFIGURACION DE MOVIMIENTO
	// ---------------------------------------------------------

	/** Velocidad de caminata (cm/s) */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Movimiento|Velocidades")
	float WalkSpeed;

	/** Velocidad de sprint (cm/s) */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Movimiento|Velocidades")
	float SprintSpeed;

	/** Velocidad de apuntado (ADS) (cm/s) */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Movimiento|Velocidades")
	float AimingSpeed;

	/** Velocidad mientras se mueve agachado (cm/s) */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Movimiento|Velocidades")
	float CrouchSpeed;

	// ---------------------------------------------------------
	// FUNCIONES DE MOVIMIENTO (llamadas desde el controlador)
	// ---------------------------------------------------------

	/**
	 * Aplica movimiento hacia adelante/atrás.
	 * @param Value Valor de entrada (-1 atrás, 1 adelante)
	 */
	UFUNCTION(BlueprintCallable, Category = "Movimiento")
	void MoveForward(float Value);

	/**
	 * Aplica movimiento lateral (strafe).
	 * @param Value Valor de entrada (-1 izquierda, 1 derecha)
	 */
	UFUNCTION(BlueprintCallable, Category = "Movimiento")
	void MoveRight(float Value);

	/**
	 * Gira la cámara horizontalmente.
	 * @param Value Valor del eje horizontal del ratón/stick
	 */
	UFUNCTION(BlueprintCallable, Category = "Cámara")
	void LookRight(float Value);

	/**
	 * Gira la cámara verticalmente.
	 * @param Value Valor del eje vertical del ratón/stick
	 */
	UFUNCTION(BlueprintCallable, Category = "Cámara")
	void LookUp(float Value);

	/** Alterna entre agachado y de pie */
	UFUNCTION(BlueprintCallable, Category = "Movimiento")
	void ToggleCrouch();

	/** Inicia el sprint si hay stamina suficiente */
	UFUNCTION(BlueprintCallable, Category = "Movimiento")
	void StartSprint();

	/** Detiene el sprint */
	UFUNCTION(BlueprintCallable, Category = "Movimiento")
	void StopSprint();

	/** Activa el apuntado (ADS) */
	UFUNCTION(BlueprintCallable, Category = "Combate")
	void StartAiming();

	/** Desactiva el apuntado */
	UFUNCTION(BlueprintCallable, Category = "Combate")
	void StopAiming();

	/** Inicia el disparo del arma activa */
	UFUNCTION(BlueprintCallable, Category = "Combate")
	void StartFiring();

	/** Detiene el disparo */
	UFUNCTION(BlueprintCallable, Category = "Combate")
	void StopFiring();

	/** Solicita recarga del arma activa */
	UFUNCTION(BlueprintCallable, Category = "Combate")
	void RequestReload();

	/**
	 * Cambia al arma de la ranura especificada.
	 * @param Slot Ranura del arma a empuñar
	 */
	UFUNCTION(BlueprintCallable, Category = "Combate")
	void SwitchToWeaponSlot(EEquipmentSlot Slot);

	/**
	 * Establece el valor de inclinación lateral.
	 * @param Value Valor normalizado (-1 a 1)
	 */
	UFUNCTION(BlueprintCallable, Category = "Movimiento")
	void SetLeanAmount(float Value);

	/** Alterna el modo de fuego del arma activa */
	UFUNCTION(BlueprintCallable, Category = "Combate")
	void ToggleFireMode();

	// ---------------------------------------------------------
	// CONSULTAS
	// ---------------------------------------------------------

	/**
	 * Obtiene el actor interactuable actualmente en rango.
	 * @return Actor bajo el cursor o nullptr
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Interacción")
	AActor* GetCurrentInteractionTarget() const;

	/** Devuelve si el personaje está vivo */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Estado")
	bool IsAlive() const;

	// ---------------------------------------------------------
	// EVENTOS DE MUERTE
	// ---------------------------------------------------------

	/**
	 * Maneja la muerte del personaje.
	 * Activa ragdoll, notifica al GameMode y deshabilita la entrada.
	 * @param Killer Actor que causó la muerte
	 */
	UFUNCTION(BlueprintCallable, Category = "Estado")
	void HandleDeath(AActor* Killer);

	// ---------------------------------------------------------
	// OVERRIDES DE ACharacter
	// ---------------------------------------------------------
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaTime) override;
	virtual void SetupPlayerInputComponent(UInputComponent* PlayerInputComponent) override;
	virtual float TakeDamage(float DamageAmount, struct FDamageEvent const& DamageEvent,
		AController* EventInstigator, AActor* DamageCauser) override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	virtual void OnRep_PlayerState() override;

protected:
	/** Velocidad de rotación de la cámara (sensibilidad) */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Cámara|Configuración")
	float BaseTurnRate;

	/** Velocidad de inclinación de la cámara */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Cámara|Configuración")
	float BaseLookUpRate;

	/** Se llama cuando el HealthComponent notifica la muerte */
	UFUNCTION()
	void OnHealthDepleted(AActor* Victim, AActor* Killer);

	/** Actualiza la velocidad de movimiento según el estado del personaje */
	void UpdateMovementSpeed();

	/** Envía el lean al servidor para replicación */
	UFUNCTION(Server, Reliable)
	void ServerSetLean(float NewLeanAmount);

	/** Envía el estado de sprint al servidor */
	UFUNCTION(Server, Reliable)
	void ServerSetSprinting(bool bNewSprinting);
};
