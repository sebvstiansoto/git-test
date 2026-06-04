// RaidPlayerState.h
// Estado del jugador replicado para incursiones en ZonaRoja
// Mantiene estadísticas, inventario y estado del jugador durante la partida

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerState.h"
#include "Data/ZRTypes.h"
#include "RaidPlayerState.generated.h"

/**
 * ARaidPlayerState
 * Almacena el estado persistente del jugador durante una incursión.
 * Replicado a todos los clientes para mostrar información en el HUD.
 */
UCLASS()
class ZONAROJA_API ARaidPlayerState : public APlayerState
{
	GENERATED_BODY()

public:
	ARaidPlayerState();

	// ---------------------------------------------------------
	// ESTADO VITAL DEL JUGADOR (replicado)
	// ---------------------------------------------------------

	/** Si el jugador sigue vivo en la incursión */
	UPROPERTY(Replicated, BlueprintReadOnly, Category = "Estado|Vital")
	bool bIsAlive;

	/** Si el jugador extrajo con éxito en esta incursión */
	UPROPERTY(Replicated, BlueprintReadOnly, Category = "Estado|Vital")
	bool bHasExtracted;

	/** Si el jugador está actualmente en una zona de extracción */
	UPROPERTY(Replicated, BlueprintReadOnly, Category = "Estado|Vital")
	bool bIsInExtractionZone;

	/** Salud actual del jugador (para mostrar en el HUD de escuadra) */
	UPROPERTY(Replicated, BlueprintReadOnly, Category = "Estado|Vital")
	float CurrentHealth;

	/** Salud máxima del jugador */
	UPROPERTY(Replicated, BlueprintReadOnly, Category = "Estado|Vital")
	float MaxHealth;

	// ---------------------------------------------------------
	// ESTADISTICAS DE LA INCURSION (replicadas)
	// ---------------------------------------------------------

	/** Número de bajas confirmadas en esta incursión */
	UPROPERTY(Replicated, BlueprintReadOnly, Category = "Estadísticas")
	int32 KillCount;

	/** Número de veces que el jugador ha muerto en incursiones (estadística total) */
	UPROPERTY(Replicated, BlueprintReadOnly, Category = "Estadísticas")
	int32 DeathCount;

	/** Daño total infligido en esta incursión */
	UPROPERTY(Replicated, BlueprintReadOnly, Category = "Estadísticas")
	float TotalDamageDealt;

	/** Daño total recibido en esta incursión */
	UPROPERTY(Replicated, BlueprintReadOnly, Category = "Estadísticas")
	float TotalDamageTaken;

	/** Objetos recogidos en esta incursión */
	UPROPERTY(Replicated, BlueprintReadOnly, Category = "Estadísticas")
	int32 ItemsLooted;

	// ---------------------------------------------------------
	// INFORMACION DEL PERFIL (replicada)
	// ---------------------------------------------------------

	/** Nivel de experiencia del jugador */
	UPROPERTY(Replicated, BlueprintReadOnly, Category = "Perfil")
	int32 PlayerLevel;

	/** Experiencia acumulada en esta incursión */
	UPROPERTY(Replicated, BlueprintReadOnly, Category = "Perfil")
	int32 ExperienceGained;

	/** Nombre de la facción a la que pertenece el jugador */
	UPROPERTY(Replicated, BlueprintReadOnly, Category = "Perfil")
	FName FactionName;

	// ---------------------------------------------------------
	// FUNCIONES DE UTILIDAD
	// ---------------------------------------------------------

	/**
	 * Agrega puntos de experiencia al jugador por una acción.
	 * @param Amount Cantidad de experiencia a otorgar
	 * @param Reason Razón de la recompensa (para registro)
	 */
	UFUNCTION(BlueprintCallable, Category = "Perfil|Experiencia")
	void AddExperience(int32 Amount, const FString& Reason);

	/**
	 * Registra una baja en las estadísticas del jugador.
	 * @param VictimName Nombre del jugador o NPC eliminado
	 */
	UFUNCTION(BlueprintCallable, Category = "Estadísticas")
	void RegisterKill(const FString& VictimName);

	/**
	 * Registra daño infligido para las estadísticas finales.
	 * @param DamageAmount Cantidad de daño infligido
	 */
	UFUNCTION(BlueprintCallable, Category = "Estadísticas")
	void RegisterDamageDealt(float DamageAmount);

	/**
	 * Registra daño recibido para las estadísticas finales.
	 * @param DamageAmount Cantidad de daño recibido
	 */
	UFUNCTION(BlueprintCallable, Category = "Estadísticas")
	void RegisterDamageTaken(float DamageAmount);

	/**
	 * Reinicia las estadísticas para una nueva incursión.
	 */
	UFUNCTION(BlueprintCallable, Category = "Estadísticas")
	void ResetRaidStats();

	// ---------------------------------------------------------
	// OVERRIDES
	// ---------------------------------------------------------
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	virtual void CopyProperties(APlayerState* PlayerState) override;
};
