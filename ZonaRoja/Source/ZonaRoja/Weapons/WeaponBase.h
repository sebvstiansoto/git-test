// WeaponBase.h
// Clase base abstracta para todas las armas en ZonaRoja
// Define la interfaz común de disparo, recarga y equipamiento

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Data/ZRTypes.h"
#include "WeaponBase.generated.h"

// Declaraciones anticipadas
class UWeaponDataAsset;
class USkeletalMeshComponent;
class UStaticMeshComponent;
class UNiagaraSystem;
class USoundBase;

// ---------------------------------------------------------
// DELEGADOS
// ---------------------------------------------------------

/** Delegado cuando el arma dispara */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnWeaponFired,
	AWeaponBase*, Weapon, const FVector&, MuzzleLocation);

/** Delegado cuando el arma comienza a recargar */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnWeaponReloadStart,
	AWeaponBase*, Weapon);

/** Delegado cuando el arma termina de recargar */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnWeaponReloadComplete,
	AWeaponBase*, Weapon);

/** Delegado cuando se queda sin munición */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnWeaponEmpty,
	AWeaponBase*, Weapon);

/**
 * AWeaponBase
 * Clase base abstracta para todas las armas del juego.
 * Hereda: AFirearmBase (armas de fuego) y armas de cuerpo a cuerpo.
 */
UCLASS(Abstract, NotBlueprintable)
class ZONAROJA_API AWeaponBase : public AActor
{
	GENERATED_BODY()

public:
	AWeaponBase();

	// ---------------------------------------------------------
	// COMPONENTES VISUALES
	// ---------------------------------------------------------

	/** Mallado del arma con sistema de animaciones */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Arma|Componentes")
	TObjectPtr<USkeletalMeshComponent> WeaponMesh;

	// ---------------------------------------------------------
	// DATOS DEL ARMA
	// ---------------------------------------------------------

	/** Asset de datos que define las estadísticas y propiedades del arma */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Arma|Datos")
	TObjectPtr<UWeaponDataAsset> WeaponData;

	/** Munición actual en el cargador */
	UPROPERTY(Replicated, BlueprintReadOnly, Category = "Arma|Munición")
	int32 CurrentAmmoInMag;

	/** Total de munición disponible fuera del cargador */
	UPROPERTY(Replicated, BlueprintReadOnly, Category = "Arma|Munición")
	int32 TotalAmmo;

	/** Categoría del arma (rifle, pistola, etc.) */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Arma|Datos")
	EWeaponCategory WeaponCategory;

	/** Tipo de munición que usa esta arma */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Arma|Datos")
	EAmmoType AmmoType;

	// ---------------------------------------------------------
	// CONFIGURACION DE MUNICION
	// ---------------------------------------------------------

	/** Capacidad máxima del cargador */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Arma|Munición")
	int32 MagCapacity;

	/** Daño base por impacto */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Arma|Estadísticas")
	float BaseDamage;

	/** Velocidad del proyectil en m/s (para cálculo de trayectoria) */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Arma|Estadísticas")
	float MuzzleVelocity;

	/** Penetración de armadura (0-100) */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Arma|Estadísticas")
	float ArmorPenetration;

	// ---------------------------------------------------------
	// ESTADO DEL ARMA
	// ---------------------------------------------------------

	/** Si el arma está actualmente equipada y sostenida */
	UPROPERTY(Replicated, BlueprintReadOnly, Category = "Arma|Estado")
	bool bIsEquipped;

	/** Si el arma está en proceso de recarga */
	UPROPERTY(Replicated, BlueprintReadOnly, Category = "Arma|Estado")
	bool bIsReloading;

	// ---------------------------------------------------------
	// EFECTOS
	// ---------------------------------------------------------

	/** Sistema de partículas Niagara para el fogonazo del disparo */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Arma|Efectos")
	TObjectPtr<UNiagaraSystem> MuzzleFlashEffect;

	/** Sonido del disparo */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Arma|Efectos")
	TObjectPtr<USoundBase> FireSound;

	/** Sonido al vaciar el cargador */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Arma|Efectos")
	TObjectPtr<USoundBase> EmptySound;

	/** Nombre del socket del cañón en el esqueleto del arma */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Arma|Sockets")
	FName MuzzleSocketName;

	// ---------------------------------------------------------
	// FUNCIONES VIRTUALES PRINCIPALES
	// ---------------------------------------------------------

	/**
	 * Inicia el disparo del arma.
	 * Implementado en las subclases según el mecanismo de disparo.
	 */
	UFUNCTION(BlueprintCallable, Category = "Arma")
	virtual void Fire();

	/**
	 * Detiene el disparo continuo (para automáticos).
	 */
	UFUNCTION(BlueprintCallable, Category = "Arma")
	virtual void StopFire();

	/**
	 * Realiza la recarga del arma.
	 */
	UFUNCTION(BlueprintCallable, Category = "Arma")
	virtual void Reload();

	/**
	 * Equipa el arma: la hace visible y activa.
	 */
	UFUNCTION(BlueprintCallable, Category = "Arma")
	virtual void Equip();

	/**
	 * Desequipa el arma: la oculta pero mantiene en el personaje.
	 */
	UFUNCTION(BlueprintCallable, Category = "Arma")
	virtual void Unequip();

	// ---------------------------------------------------------
	// RPC SERVIDOR
	// ---------------------------------------------------------

	/**
	 * RPC de servidor para validar el disparo.
	 * El cliente envía la información del disparo para confirmación server-side.
	 * @param MuzzleLocation Posición del cañón en el momento del disparo
	 * @param ShotRotation Rotación del disparo (dirección)
	 */
	UFUNCTION(Server, Reliable)
	void ServerFire(FVector MuzzleLocation, FRotator ShotRotation);

	// ---------------------------------------------------------
	// CONSULTAS
	// ---------------------------------------------------------

	/** Devuelve si el arma tiene munición en el cargador */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Arma|Munición")
	bool HasAmmoInMag() const { return CurrentAmmoInMag > 0; }

	/** Devuelve si hay munición de reserva para recargar */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Arma|Munición")
	bool HasReserveAmmo() const { return TotalAmmo > 0; }

	/** Devuelve si el arma puede disparar ahora */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Arma")
	virtual bool CanFire() const;

	/** Obtiene la posición del cañón en el mundo */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Arma")
	FVector GetMuzzleLocation() const;

	/** Obtiene la rotación del cañón en el mundo */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Arma")
	FRotator GetMuzzleRotation() const;

	// ---------------------------------------------------------
	// DELEGADOS
	// ---------------------------------------------------------

	UPROPERTY(BlueprintAssignable, Category = "Arma|Delegados")
	FOnWeaponFired OnWeaponFired;

	UPROPERTY(BlueprintAssignable, Category = "Arma|Delegados")
	FOnWeaponReloadStart OnWeaponReloadStart;

	UPROPERTY(BlueprintAssignable, Category = "Arma|Delegados")
	FOnWeaponReloadComplete OnWeaponReloadComplete;

	UPROPERTY(BlueprintAssignable, Category = "Arma|Delegados")
	FOnWeaponEmpty OnWeaponEmpty;

	// ---------------------------------------------------------
	// OVERRIDES
	// ---------------------------------------------------------
	virtual void BeginPlay() override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

protected:
	/**
	 * Reproduce los efectos del disparo (fogonazo, sonido, animación).
	 * Se llama localmente en todos los clientes mediante Multicast.
	 */
	UFUNCTION(NetMulticast, Unreliable)
	void MulticastPlayFireEffects(FVector MuzzleLocation);

	/** Consume una unidad de munición del cargador */
	void ConsumeAmmo();

	/** Procesa la recarga: mueve la munición de reserva al cargador */
	void ProcessReload();
};
