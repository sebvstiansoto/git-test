// RaidGameMode.h
// Modo de juego principal para incursiones en ZonaRoja
// Controla el ciclo de vida de la partida, temporizadores y condiciones de victoria

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameMode.h"
#include "Data/ZRTypes.h"
#include "RaidGameMode.generated.h"

// Declaraciones anticipadas para evitar dependencias circulares
class ARaidGameState;
class ARaidPlayerState;
class ARaidPlayerController;

/** Delegado para notificar cambios en la fase de la incursión */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnRaidPhaseChanged, ERaidPhase, OldPhase, ERaidPhase, NewPhase);

/** Delegado para notificar la extracción exitosa de un jugador */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnPlayerExtracted, ARaidPlayerState*, PlayerState);

/** Delegado para notificar la muerte de un jugador */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnPlayerDied, ARaidPlayerState*, VictimState, ARaidPlayerState*, KillerState);

/**
 * ARaidGameMode
 * Modo de juego principal que gestiona el ciclo de las incursiones.
 * Solo existe en el servidor. Controla el tiempo, fases y extracción.
 */
UCLASS(Config = Game)
class ZONAROJA_API ARaidGameMode : public AGameMode
{
	GENERATED_BODY()

public:
	// ---------------------------------------------------------
	// CONSTRUCTOR
	// ---------------------------------------------------------
	ARaidGameMode();

	// ---------------------------------------------------------
	// CICLO DE VIDA DE LA INCURSION
	// ---------------------------------------------------------

	/** Inicia la fase de inserción al comenzar la partida */
	UFUNCTION(BlueprintCallable, Category = "Incursión|Fase")
	void StartInsertionPhase();

	/** Transiciona a la fase activa cuando todos los jugadores han insertado */
	UFUNCTION(BlueprintCallable, Category = "Incursión|Fase")
	void StartActivePhase();

	/** Inicia la cuenta regresiva de evacuación */
	UFUNCTION(BlueprintCallable, Category = "Incursión|Fase")
	void StartEvacuationPhase();

	/** Finaliza la incursión y procesa los resultados */
	UFUNCTION(BlueprintCallable, Category = "Incursión|Fase")
	void EndRaid();

	/** Cambia la fase actual de la incursión */
	UFUNCTION(BlueprintCallable, Category = "Incursión|Fase")
	void SetRaidPhase(ERaidPhase NewPhase);

	// ---------------------------------------------------------
	// GESTION DE JUGADORES
	// ---------------------------------------------------------

	/**
	 * Procesa la solicitud de extracción de un jugador.
	 * Verifica que el jugador esté en una zona válida y tenga el ítem necesario.
	 * @param PlayerController Controlador del jugador que intenta extraer
	 */
	UFUNCTION(BlueprintCallable, Category = "Incursión|Extracción")
	void RequestPlayerExtraction(ARaidPlayerController* PlayerController);

	/**
	 * Confirma la extracción exitosa de un jugador.
	 * Guarda el inventario y estadísticas del jugador.
	 * @param PlayerController Controlador del jugador extraído
	 */
	UFUNCTION(BlueprintCallable, Category = "Incursión|Extracción")
	void ConfirmPlayerExtraction(ARaidPlayerController* PlayerController);

	/**
	 * Maneja la muerte de un jugador durante la incursión.
	 * Determina si spawner o si pierde su inventario (modo hardcore).
	 * @param VictimController Controlador del jugador muerto
	 * @param KillerController Controlador del jugador asesino (puede ser nulo)
	 */
	UFUNCTION(BlueprintCallable, Category = "Incursión|Jugadores")
	void HandlePlayerDeath(ARaidPlayerController* VictimController, ARaidPlayerController* KillerController);

	/** Verifica si quedan condiciones para continuar la partida */
	UFUNCTION(BlueprintCallable, Category = "Incursión|Condiciones")
	bool ShouldEndRaid() const;

	// ---------------------------------------------------------
	// ZONAS DE EXTRACCION
	// ---------------------------------------------------------

	/**
	 * Activa una zona de extracción en el mapa.
	 * @param ZoneID Identificador único de la zona
	 */
	UFUNCTION(BlueprintCallable, Category = "Incursión|Zonas")
	void ActivateExtractionZone(FName ZoneID);

	/**
	 * Desactiva una zona de extracción (puede ser temporal).
	 * @param ZoneID Identificador único de la zona
	 */
	UFUNCTION(BlueprintCallable, Category = "Incursión|Zonas")
	void DeactivateExtractionZone(FName ZoneID);

	/** Selecciona aleatoriamente las zonas activas para esta incursión */
	UFUNCTION(BlueprintCallable, Category = "Incursión|Zonas")
	void RandomizeActiveExtractionZones();

	// ---------------------------------------------------------
	// OVERRIDES DE AGameMode
	// ---------------------------------------------------------
	virtual void InitGame(const FString& MapName, const FString& Options, FString& ErrorMessage) override;
	virtual void BeginPlay() override;
	virtual void PostLogin(APlayerController* NewPlayer) override;
	virtual void Logout(AController* Exiting) override;
	virtual AActor* ChoosePlayerStart_Implementation(AController* Player) override;

	// ---------------------------------------------------------
	// CONFIGURACION
	// ---------------------------------------------------------

	/** Duración total de la incursión en segundos (45 minutos por defecto) */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Configuración|Tiempo")
	float RaidDuration;

	/** Duración de la fase de evacuación en segundos */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Configuración|Tiempo")
	float EvacuationDuration;

	/** Número máximo de jugadores por incursión */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Configuración|Jugadores")
	int32 MaxPlayersPerRaid;

	/** Número de zonas de extracción activas simultáneamente */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Configuración|Extracción")
	int32 ActiveExtractionZones;

	/** Si está habilitado el modo hardcore (pérdida de inventario al morir) */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Configuración|Jugabilidad")
	bool bHardcoreMode;

	// ---------------------------------------------------------
	// DELEGADOS
	// ---------------------------------------------------------

	/** Notifica cuando la fase de la incursión cambia */
	UPROPERTY(BlueprintAssignable, Category = "Incursión|Delegados")
	FOnRaidPhaseChanged OnRaidPhaseChanged;

	/** Notifica cuando un jugador extrae con éxito */
	UPROPERTY(BlueprintAssignable, Category = "Incursión|Delegados")
	FOnPlayerExtracted OnPlayerExtracted;

	/** Notifica cuando un jugador muere */
	UPROPERTY(BlueprintAssignable, Category = "Incursión|Delegados")
	FOnPlayerDied OnPlayerDied;

protected:
	// ---------------------------------------------------------
	// ESTADO INTERNO
	// ---------------------------------------------------------

	/** Fase actual de la incursión */
	UPROPERTY(VisibleInstanceOnly, Category = "Estado")
	ERaidPhase CurrentPhase;

	/** Handle del temporizador principal de la incursión */
	FTimerHandle RaidTimerHandle;

	/** Handle del temporizador de tick de la incursión */
	FTimerHandle RaidTickHandle;

	/** Tick interno de la incursión (cada segundo actualiza GameState) */
	void RaidTimerTick();

	/** Llamado cuando el temporizador de la incursión llega a cero */
	void OnRaidTimerExpired();

	/** Obtiene el estado de juego de la incursión con tipo correcto */
	ARaidGameState* GetRaidGameState() const;
};
