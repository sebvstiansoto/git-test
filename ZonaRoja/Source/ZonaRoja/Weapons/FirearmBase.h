// FirearmBase.h
// Clase base para armas de fuego con hitscan y fuego automático/ráfaga
// Hereda de WeaponBase y añade mecánicas balísticas

#pragma once

#include "CoreMinimal.h"
#include "Weapons/WeaponBase.h"
#include "FirearmBase.generated.h"

// ---------------------------------------------------------
// ENUMERADO DE MODO DE FUEGO
// ---------------------------------------------------------

/** Modos de fuego disponibles en las armas de fuego */
UENUM(BlueprintType)
enum class EFireMode : uint8
{
	SemiAuto	UMETA(DisplayName = "Semiautomático"),	// Un disparo por pulsación
	Burst		UMETA(DisplayName = "Ráfaga"),			// 3 disparos por pulsación
	FullAuto	UMETA(DisplayName = "Automático")		// Fuego continuo al mantener
};

/**
 * AFirearmBase
 * Clase base para todas las armas de fuego del juego.
 * Implementa el sistema de hitscan con compensación de lag,
 * disparo automático mediante timer y ráfagas de 3 balas.
 */
UCLASS(Abstract, Blueprintable)
class ZONAROJA_API AFirearmBase : public AWeaponBase
{
	GENERATED_BODY()

public:
	AFirearmBase();

	// ---------------------------------------------------------
	// CONFIGURACION DE DISPARO
	// ---------------------------------------------------------

	/** Cadencia de disparo en Rounds Per Minute (RPM) */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Arma de Fuego|Cadencia")
	float RoundsPerMinute;

	/** Modos de fuego disponibles para este arma */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Arma de Fuego|Modos")
	TArray<EFireMode> AvailableFireModes;

	/** Modo de fuego actualmente seleccionado */
	UPROPERTY(Replicated, BlueprintReadOnly, Category = "Arma de Fuego|Modos")
	EFireMode CurrentFireMode;

	/** Número de balas por ráfaga (cuando está en modo Burst) */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Arma de Fuego|Ráfaga")
	int32 BurstCount;

	// ---------------------------------------------------------
	// CONFIGURACION BALISTICA
	// ---------------------------------------------------------

	/** Alcance máximo del hitscan en centímetros */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Arma de Fuego|Balística")
	float MaxHitscanRange;

	/** Dispersión base en reposo (en grados de desvío del cono) */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Arma de Fuego|Balística")
	float BaseSpreadAngle;

	/** Dispersión máxima al esprintar o en movimiento intenso */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Arma de Fuego|Balística")
	float MaxSpreadAngle;

	/** Incremento de dispersión por disparo (retroceso acumulado) */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Arma de Fuego|Balística")
	float SpreadIncreasePerShot;

	/** Velocidad de recuperación de la dispersión (grados/segundo) */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Arma de Fuego|Balística")
	float SpreadRecoveryRate;

	/** Reducción de dispersión al apuntar (multiplicador, ej. 0.25 = 75% menos dispersión) */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Arma de Fuego|Balística")
	float ADSSpreadMultiplier;

	/** Número de proyectiles por disparo (> 1 para escopetas) */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Arma de Fuego|Balística")
	int32 ProjectilesPerShot;

	// ---------------------------------------------------------
	// ESTADO DEL DISPARO
	// ---------------------------------------------------------

	/** Dispersión actual acumulada por disparos consecutivos */
	UPROPERTY(BlueprintReadOnly, Category = "Arma de Fuego|Estado")
	float CurrentSpread;

	/** Si el jugador está apuntando (ADS) */
	UPROPERTY(Replicated, BlueprintReadOnly, Category = "Arma de Fuego|Estado")
	bool bIsAiming;

	/** Contador de balas disparadas en la ráfaga actual */
	int32 BurstShotsRemaining;

	// ---------------------------------------------------------
	// OVERRIDES
	// ---------------------------------------------------------
	virtual void Fire() override;
	virtual void StopFire() override;
	virtual bool CanFire() const override;
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaTime) override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	// ---------------------------------------------------------
	// FUNCIONES DE CONTROL
	// ---------------------------------------------------------

	/**
	 * Alterna al siguiente modo de fuego disponible.
	 * Cicla entre los modos en AvailableFireModes.
	 */
	UFUNCTION(BlueprintCallable, Category = "Arma de Fuego")
	void CycleFireMode();

	/**
	 * Activa o desactiva el apuntado (ADS).
	 * @param bAiming Si debe activar el ADS
	 */
	UFUNCTION(BlueprintCallable, Category = "Arma de Fuego")
	void SetAiming(bool bAiming);

	/**
	 * Obtiene el intervalo entre disparos en segundos basado en RPM.
	 * @return Intervalo en segundos entre disparos
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Arma de Fuego")
	float GetFireInterval() const;

protected:
	/** Timer para el disparo automático */
	FTimerHandle FireTimerHandle;

	/** Timer para recuperación de la dispersión */
	FTimerHandle SpreadRecoveryTimerHandle;

	/** Si el trigger está presionado (para fuego automático) */
	bool bTriggerHeld;

	/** Tiempo del último disparo para calcular cadencia */
	float LastFireTime;

	/**
	 * Ejecuta un disparo hitscan desde el cañón del arma.
	 * Lanza un raycast y aplica daño al primer actor impactado.
	 */
	void PerformHitscanShot();

	/**
	 * Calcula la dirección del disparo aplicando la dispersión actual.
	 * @param BaseDirection Dirección base (normalmente la del cañón)
	 * @return Dirección con dispersión aplicada
	 */
	FVector ApplySpreadToDirection(const FVector& BaseDirection) const;

	/**
	 * Procesa el impacto del hitscan en el servidor.
	 * Aplica daño al actor impactado si es un personaje.
	 * @param HitResult Resultado del raycast
	 */
	void ProcessHitResult(const FHitResult& HitResult);

	/** Llamado por el timer de fuego automático */
	void AutoFireTick();

	/** Inicia la recuperación de dispersión después de parar de disparar */
	void StartSpreadRecovery();

	/** Tick de recuperación de dispersión */
	void SpreadRecoveryTick();

	/**
	 * Spawn efectos de impacto en la superficie golpeada.
	 * @param HitResult Información del impacto
	 */
	UFUNCTION(NetMulticast, Unreliable)
	void MulticastPlayImpactEffects(FHitResult HitResult);
};
