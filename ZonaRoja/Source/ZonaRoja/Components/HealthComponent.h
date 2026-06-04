// HealthComponent.h
// Componente de salud con sistema de daño localizado por partes del cuerpo
// Soporta armadura, sangrado, fracturas y múltiples tipos de daño

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Data/ZRTypes.h"
#include "HealthComponent.generated.h"

// Declaraciones anticipadas
class UDamageType;

// ---------------------------------------------------------
// DELEGADOS
// ---------------------------------------------------------

/** Delegado cuando el actor muere */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnDeath,
	AActor*, Victim, AActor*, Killer);

/** Delegado cuando se recibe daño en una parte específica */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_FiveParams(FOnDamageReceived,
	float, DamageAmount, EBodyPart, HitBodyPart, EDamageType, DamageType,
	AActor*, DamageInstigator, const FHitResult&, HitResult);

/** Delegado cuando el estado de sangrado cambia */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnBleedingStateChanged,
	EBodyPart, AffectedPart, bool, bIsBleeding);

/** Delegado cuando se aplica curación */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnHealed,
	float, HealAmount, EBodyPart, HealedPart);

/**
 * UHealthComponent
 * Componente que gestiona el sistema de salud por partes del cuerpo.
 * Implementa daño localizado, armadura por zona, sangrado y regeneración.
 * Diseñado para jugadores y NPCs en el shooter de extracción.
 */
UCLASS(ClassGroup = (ZonaRoja), meta = (BlueprintSpawnableComponent))
class ZONAROJA_API UHealthComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UHealthComponent();

	// ---------------------------------------------------------
	// CONFIGURACION
	// ---------------------------------------------------------

	/** Salud máxima de cada parte del cuerpo por defecto */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Salud|Configuración")
	float DefaultPartMaxHealth;

	/** Multiplicador de daño para impactos en la cabeza */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Salud|Modificadores")
	float HeadDamageMultiplier;

	/** Multiplicador de daño para impactos en el pecho */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Salud|Modificadores")
	float ChestDamageMultiplier;

	/** Multiplicador de daño para impactos en el estómago */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Salud|Modificadores")
	float StomachDamageMultiplier;

	/** Multiplicador de daño para impactos en extremidades */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Salud|Modificadores")
	float LimbDamageMultiplier;

	/** Si el actor puede morir (false para actores invulnerables en testing) */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Salud|Configuración")
	bool bCanDie;

	// ---------------------------------------------------------
	// ESTADO ACTUAL (replicado)
	// ---------------------------------------------------------

	/** Salud de cada parte del cuerpo */
	UPROPERTY(ReplicatedUsing = OnRep_BodyPartHealthData, BlueprintReadOnly, Category = "Salud|Estado")
	TArray<FBodyPartHealth> BodyPartHealthData;

	/** Si el actor está muerto */
	UPROPERTY(Replicated, BlueprintReadOnly, Category = "Salud|Estado")
	bool bIsDead;

	// ---------------------------------------------------------
	// FUNCIONES PRINCIPALES
	// ---------------------------------------------------------

	/**
	 * Aplica daño a una parte específica del cuerpo.
	 * Calcula penetración de armadura y modificadores de zona.
	 * @param BaseDamage Daño base antes de modificadores
	 * @param HitBodyPart Parte del cuerpo impactada
	 * @param DamageType Tipo de daño para calcular modificadores
	 * @param DamageInstigator Actor que causó el daño
	 * @param HitResult Información del impacto para efectos visuales
	 */
	UFUNCTION(BlueprintCallable, Category = "Salud")
	void ApplyDamage(float BaseDamage, EBodyPart HitBodyPart, EDamageType DamageType,
		AActor* DamageInstigator, const FHitResult& HitResult);

	/**
	 * Aplica curación a una parte específica del cuerpo.
	 * @param HealAmount Cantidad de salud a restaurar
	 * @param TargetPart Parte del cuerpo a curar
	 */
	UFUNCTION(BlueprintCallable, Category = "Salud")
	void HealBodyPart(float HealAmount, EBodyPart TargetPart);

	/**
	 * Aplica curación a todas las partes del cuerpo proporcionalmente.
	 * @param TotalHealAmount Cantidad total de salud a distribuir
	 */
	UFUNCTION(BlueprintCallable, Category = "Salud")
	void HealAll(float TotalHealAmount);

	/**
	 * Aplica armadura a una parte del cuerpo.
	 * @param ArmorValue Valor de armadura a añadir
	 * @param TargetPart Parte del cuerpo a proteger
	 */
	UFUNCTION(BlueprintCallable, Category = "Salud")
	void ApplyArmor(float ArmorValue, EBodyPart TargetPart);

	/**
	 * Inicia el sangrado en una parte del cuerpo.
	 * @param BleedRate Daño por segundo del sangrado
	 * @param AffectedPart Parte que sangra
	 */
	UFUNCTION(BlueprintCallable, Category = "Salud|Sangrado")
	void StartBleeding(float BleedRate, EBodyPart AffectedPart);

	/**
	 * Detiene el sangrado en una parte del cuerpo.
	 * @param AffectedPart Parte a curar del sangrado
	 */
	UFUNCTION(BlueprintCallable, Category = "Salud|Sangrado")
	void StopBleeding(EBodyPart AffectedPart);

	// ---------------------------------------------------------
	// CONSULTAS
	// ---------------------------------------------------------

	/**
	 * Obtiene la salud actual de una parte del cuerpo.
	 * @param BodyPart Parte del cuerpo a consultar
	 * @return Salud actual de esa parte
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Salud")
	float GetBodyPartHealth(EBodyPart BodyPart) const;

	/**
	 * Obtiene la salud total sumando todas las partes.
	 * @return Suma de salud de todas las partes del cuerpo
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Salud")
	float GetTotalHealth() const;

	/**
	 * Obtiene la salud total máxima.
	 * @return Suma de salud máxima de todas las partes
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Salud")
	float GetTotalMaxHealth() const;

	/**
	 * Devuelve si el actor está muerto.
	 * @return true si el actor está muerto
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Salud")
	bool IsDead() const { return bIsDead; }

	/**
	 * Devuelve si alguna parte del cuerpo está sangrando.
	 * @return true si hay sangrado activo
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Salud")
	bool IsAnyPartBleeding() const;

	// ---------------------------------------------------------
	// DELEGADOS PUBLICOS
	// ---------------------------------------------------------

	/** Notifica cuando el actor muere */
	UPROPERTY(BlueprintAssignable, Category = "Salud|Delegados")
	FOnDeath OnDeath;

	/** Notifica cuando se recibe daño */
	UPROPERTY(BlueprintAssignable, Category = "Salud|Delegados")
	FOnDamageReceived OnDamageReceived;

	/** Notifica cambios en el estado de sangrado */
	UPROPERTY(BlueprintAssignable, Category = "Salud|Delegados")
	FOnBleedingStateChanged OnBleedingStateChanged;

	/** Notifica cuando se aplica curación */
	UPROPERTY(BlueprintAssignable, Category = "Salud|Delegados")
	FOnHealed OnHealed;

	// ---------------------------------------------------------
	// OVERRIDES
	// ---------------------------------------------------------
	virtual void BeginPlay() override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

protected:
	/** Inicializa los datos de salud de todas las partes del cuerpo */
	void InitializeBodyParts();

	/**
	 * Calcula el daño final después de aplicar armadura y modificadores.
	 * @param RawDamage Daño sin modificar
	 * @param Part Parte del cuerpo impactada
	 * @param DmgType Tipo de daño para el cálculo
	 * @return Daño final a aplicar a la salud
	 */
	float CalculateFinalDamage(float RawDamage, const FBodyPartHealth& Part, EDamageType DmgType) const;

	/**
	 * Obtiene el multiplicador de daño para una parte del cuerpo.
	 * @param BodyPart Parte del cuerpo
	 * @return Multiplicador a aplicar al daño base
	 */
	float GetBodyPartDamageMultiplier(EBodyPart BodyPart) const;

	/**
	 * Obtiene el índice de una parte del cuerpo en el array.
	 * @param BodyPart Parte del cuerpo a buscar
	 * @return Índice en el array, o -1 si no existe
	 */
	int32 GetBodyPartIndex(EBodyPart BodyPart) const;

	/** Procesa la muerte del actor */
	void ProcessDeath(AActor* Killer);

	/** Tick de sangrado (se llama cada segundo) */
	void BleedingTick();

	/** Callback de replicación de datos de partes del cuerpo */
	UFUNCTION()
	void OnRep_BodyPartHealthData();

	/** Handle para el timer de sangrado */
	FTimerHandle BleedingTimerHandle;
};
