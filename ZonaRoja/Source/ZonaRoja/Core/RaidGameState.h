// RaidGameState.h
// Estado de juego replicado para incursiones en ZonaRoja
// Contiene toda la información visible para todos los clientes

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameState.h"
#include "Data/ZRTypes.h"
#include "RaidGameState.generated.h"

/**
 * ARaidGameState
 * Estado de juego replicado que mantiene información global de la incursión.
 * Los clientes reciben actualizaciones automáticas de estas propiedades.
 */
UCLASS()
class ZONAROJA_API ARaidGameState : public AGameState
{
	GENERATED_BODY()

public:
	ARaidGameState();

	// ---------------------------------------------------------
	// PROPIEDADES REPLICADAS - TIEMPO
	// ---------------------------------------------------------

	/** Tiempo restante de la incursión en segundos */
	UPROPERTY(ReplicatedUsing = OnRep_RaidTimeRemaining, BlueprintReadOnly, Category = "Incursión|Tiempo")
	float RaidTimeRemaining;

	/** Fase actual de la incursión (replicada a todos los clientes) */
	UPROPERTY(ReplicatedUsing = OnRep_CurrentRaidPhase, BlueprintReadOnly, Category = "Incursión|Fase")
	ERaidPhase CurrentRaidPhase;

	// ---------------------------------------------------------
	// PROPIEDADES REPLICADAS - JUGADORES
	// ---------------------------------------------------------

	/** Número total de jugadores vivos en la incursión */
	UPROPERTY(Replicated, BlueprintReadOnly, Category = "Incursión|Jugadores")
	int32 TotalPlayersAlive;

	/** Número total de jugadores que han extraído con éxito */
	UPROPERTY(Replicated, BlueprintReadOnly, Category = "Incursión|Jugadores")
	int32 TotalPlayersExtracted;

	// ---------------------------------------------------------
	// PROPIEDADES REPLICADAS - ZONAS DE EXTRACCION
	// ---------------------------------------------------------

	/** Lista de zonas de extracción y su estado actual */
	UPROPERTY(ReplicatedUsing = OnRep_ActiveExtractionZonesList, BlueprintReadOnly, Category = "Incursión|Zonas")
	TArray<FExtractionZoneInfo> ActiveExtractionZonesList;

	// ---------------------------------------------------------
	// FUNCIONES DE REPLICACION (OnRep)
	// ---------------------------------------------------------

	/** Llamado cuando el tiempo restante cambia */
	UFUNCTION()
	void OnRep_RaidTimeRemaining();

	/** Llamado cuando la fase de la incursión cambia */
	UFUNCTION()
	void OnRep_CurrentRaidPhase();

	/** Llamado cuando las zonas de extracción cambian */
	UFUNCTION()
	void OnRep_ActiveExtractionZonesList();

	// ---------------------------------------------------------
	// FUNCIONES DE UTILIDAD
	// ---------------------------------------------------------

	/**
	 * Obtiene el tiempo restante formateado como MM:SS para el HUD.
	 * @return Texto con formato "MM:SS"
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Incursión|UI")
	FText GetFormattedTimeRemaining() const;

	/**
	 * Obtiene el nombre visible de la fase actual.
	 * @return Texto localizado con el nombre de la fase
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Incursión|UI")
	FText GetCurrentPhaseDisplayName() const;

	/**
	 * Busca una zona de extracción por su ID.
	 * @param ZoneID Identificador de la zona
	 * @param OutZone Estructura con la información de la zona
	 * @return true si se encontró la zona
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Incursión|Zonas")
	bool GetExtractionZoneByID(FName ZoneID, FExtractionZoneInfo& OutZone) const;

	// ---------------------------------------------------------
	// OVERRIDES
	// ---------------------------------------------------------
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
};
