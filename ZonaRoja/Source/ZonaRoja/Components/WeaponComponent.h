// WeaponComponent.h
// Componente de gestión de armas del personaje en ZonaRoja
// Maneja el equipamiento, cambio y estado de las armas

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Data/ZRTypes.h"
#include "WeaponComponent.generated.h"

// Declaraciones anticipadas
class AWeaponBase;
class AFirearmBase;

// ---------------------------------------------------------
// DELEGADOS
// ---------------------------------------------------------

/** Delegado cuando se equipa un arma */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnWeaponEquipped,
	AWeaponBase*, NewWeapon, EEquipmentSlot, Slot);

/** Delegado cuando se desequipa un arma */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnWeaponUnequipped,
	EEquipmentSlot, Slot);

/** Delegado cuando se inicia la recarga */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnReloadStarted,
	AWeaponBase*, Weapon);

/** Delegado cuando termina la recarga */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnReloadCompleted,
	AWeaponBase*, Weapon);

/**
 * UWeaponComponent
 * Gestiona todas las armas que lleva el personaje.
 * Controla qué arma está activa, el cambio entre ellas y las animaciones.
 */
UCLASS(ClassGroup = (ZonaRoja), meta = (BlueprintSpawnableComponent))
class ZONAROJA_API UWeaponComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UWeaponComponent();

	// ---------------------------------------------------------
	// ESTADO DE ARMAS (replicado)
	// ---------------------------------------------------------

	/** Arma actualmente empuñada */
	UPROPERTY(ReplicatedUsing = OnRep_CurrentWeapon, BlueprintReadOnly, Category = "Armas|Estado")
	TObjectPtr<AWeaponBase> CurrentWeapon;

	/** Arma equipada en la ranura primaria */
	UPROPERTY(Replicated, BlueprintReadOnly, Category = "Armas|Estado")
	TObjectPtr<AWeaponBase> PrimaryWeapon;

	/** Arma equipada en la ranura secundaria */
	UPROPERTY(Replicated, BlueprintReadOnly, Category = "Armas|Estado")
	TObjectPtr<AWeaponBase> SecondaryWeapon;

	/** Pistola en la pistolera */
	UPROPERTY(Replicated, BlueprintReadOnly, Category = "Armas|Estado")
	TObjectPtr<AWeaponBase> HolsterWeapon;

	/** Ranura activa actualmente */
	UPROPERTY(Replicated, BlueprintReadOnly, Category = "Armas|Estado")
	EEquipmentSlot ActiveWeaponSlot;

	/** Si el personaje está actualmente en proceso de cambio de arma */
	UPROPERTY(Replicated, BlueprintReadOnly, Category = "Armas|Estado")
	bool bIsSwappingWeapon;

	/** Si el personaje está actualmente recargando */
	UPROPERTY(Replicated, BlueprintReadOnly, Category = "Armas|Estado")
	bool bIsReloading;

	// ---------------------------------------------------------
	// FUNCIONES PRINCIPALES
	// ---------------------------------------------------------

	/**
	 * Spawn y equipa un arma en la ranura especificada.
	 * @param WeaponClass Clase del arma a spawnar
	 * @param Slot Ranura donde equipar el arma
	 * @return Referencia al arma spawneada o nullptr si falló
	 */
	UFUNCTION(BlueprintCallable, Category = "Armas")
	AWeaponBase* SpawnAndEquipWeapon(TSubclassOf<AWeaponBase> WeaponClass, EEquipmentSlot Slot);

	/**
	 * Desequipa un arma de una ranura y la destruye del mundo.
	 * @param Slot Ranura del arma a desequipar
	 */
	UFUNCTION(BlueprintCallable, Category = "Armas")
	void UnequipWeapon(EEquipmentSlot Slot);

	/**
	 * Cambia al arma de la ranura especificada.
	 * Incluye la animación de cambio de arma.
	 * @param Slot Ranura del arma a la que cambiar
	 */
	UFUNCTION(BlueprintCallable, Category = "Armas")
	void SwitchToWeaponSlot(EEquipmentSlot Slot);

	/** Inicia el disparo con el arma activa */
	UFUNCTION(BlueprintCallable, Category = "Armas")
	void StartFiring();

	/** Detiene el disparo con el arma activa */
	UFUNCTION(BlueprintCallable, Category = "Armas")
	void StopFiring();

	/** Inicia la recarga del arma activa */
	UFUNCTION(BlueprintCallable, Category = "Armas")
	void StartReload();

	/** Cancela la recarga en curso (p.ej. al recibir daño) */
	UFUNCTION(BlueprintCallable, Category = "Armas")
	void CancelReload();

	/** Cambia el modo de fuego del arma activa (semiautomático/automático/ráfaga) */
	UFUNCTION(BlueprintCallable, Category = "Armas")
	void ToggleFireMode();

	// ---------------------------------------------------------
	// CONSULTAS
	// ---------------------------------------------------------

	/** Devuelve si el personaje puede disparar en este momento */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Armas")
	bool CanFire() const;

	/** Devuelve si el personaje puede recargar en este momento */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Armas")
	bool CanReload() const;

	/** Obtiene el arma actualmente empuñada */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Armas")
	AWeaponBase* GetCurrentWeapon() const { return CurrentWeapon; }

	/** Obtiene el arma de una ranura específica */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Armas")
	AWeaponBase* GetWeaponInSlot(EEquipmentSlot Slot) const;

	// ---------------------------------------------------------
	// DELEGADOS
	// ---------------------------------------------------------

	UPROPERTY(BlueprintAssignable, Category = "Armas|Delegados")
	FOnWeaponEquipped OnWeaponEquipped;

	UPROPERTY(BlueprintAssignable, Category = "Armas|Delegados")
	FOnWeaponUnequipped OnWeaponUnequipped;

	UPROPERTY(BlueprintAssignable, Category = "Armas|Delegados")
	FOnReloadStarted OnReloadStarted;

	UPROPERTY(BlueprintAssignable, Category = "Armas|Delegados")
	FOnReloadCompleted OnReloadCompleted;

	// ---------------------------------------------------------
	// OVERRIDES
	// ---------------------------------------------------------
	virtual void BeginPlay() override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

protected:
	/** Nombre del socket del esqueleto para adjuntar armas (mano derecha) */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Armas|Sockets")
	FName WeaponSocketName;

	/** Nombre del socket para la espalda (arma primaria guardada) */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Armas|Sockets")
	FName BackSocketName;

	/** Nombre del socket para el lateral (pistolera) */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Armas|Sockets")
	FName HolsterSocketName;

	/** Handle del timer para el cambio de arma con delay (animación) */
	FTimerHandle WeaponSwitchTimer;

	/** Handle del timer para completar la recarga */
	FTimerHandle ReloadTimer;

	/** Completa el proceso de cambio de arma (después de la animación) */
	void FinishWeaponSwitch(EEquipmentSlot NewSlot);

	/** Completa la recarga (después de la animación) */
	void FinishReload();

	/** Adjunta el arma al socket correcto del esqueleto */
	void AttachWeaponToSocket(AWeaponBase* Weapon, FName SocketName);

	/** Callback de replicación del arma actual */
	UFUNCTION()
	void OnRep_CurrentWeapon();
};
