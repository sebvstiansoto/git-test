// RaidGameMode.cpp
// Implementación del modo de juego de incursión para ZonaRoja

#include "Core/RaidGameMode.h"
#include "Core/RaidGameState.h"
#include "Core/RaidPlayerState.h"
#include "Core/RaidPlayerController.h"
#include "Characters/Player/ZRPlayerCharacter.h"
#include "EngineUtils.h"
#include "GameFramework/PlayerStart.h"
#include "Kismet/GameplayStatics.h"

ARaidGameMode::ARaidGameMode()
{
	// Asignar las clases predeterminadas del modo de juego
	GameStateClass = ARaidGameState::StaticClass();
	PlayerStateClass = ARaidPlayerState::StaticClass();
	PlayerControllerClass = ARaidPlayerController::StaticClass();
	DefaultPawnClass = AZRPlayerCharacter::StaticClass();

	// Configuración inicial de la incursión
	// 2700 segundos = 45 minutos de incursión
	RaidDuration = 2700.0f;
	EvacuationDuration = 600.0f;    // 10 minutos de evacuación
	MaxPlayersPerRaid = 12;
	ActiveExtractionZones = 3;
	bHardcoreMode = true;           // Modo hardcore por defecto: pérdida de inventario

	// Fase inicial al arrancar
	CurrentPhase = ERaidPhase::Insercion;
}

void ARaidGameMode::InitGame(const FString& MapName, const FString& Options, FString& ErrorMessage)
{
	Super::InitGame(MapName, Options, ErrorMessage);

	// Parsear opciones de la URL si se pasan configuraciones dinámicas
	const FString HardcoreOption = UGameplayStatics::ParseOption(Options, TEXT("Hardcore"));
	if (!HardcoreOption.IsEmpty())
	{
		bHardcoreMode = (HardcoreOption == TEXT("1") || HardcoreOption.ToLower() == TEXT("true"));
	}

	UE_LOG(LogTemp, Log, TEXT("[ZonaRoja] InitGame - Mapa: %s | Hardcore: %s"),
		*MapName, bHardcoreMode ? TEXT("SI") : TEXT("NO"));
}

void ARaidGameMode::BeginPlay()
{
	Super::BeginPlay();

	// Seleccionar las zonas de extracción activas para esta incursión
	RandomizeActiveExtractionZones();

	// Iniciar la fase de inserción cuando comience el nivel
	StartInsertionPhase();
}

void ARaidGameMode::PostLogin(APlayerController* NewPlayer)
{
	Super::PostLogin(NewPlayer);

	UE_LOG(LogTemp, Log, TEXT("[ZonaRoja] Jugador conectado: %s"), *NewPlayer->GetName());

	// Notificar al GameState del nuevo jugador
	if (ARaidGameState* GS = GetRaidGameState())
	{
		GS->TotalPlayersAlive++;
	}
}

void ARaidGameMode::Logout(AController* Exiting)
{
	Super::Logout(Exiting);

	UE_LOG(LogTemp, Log, TEXT("[ZonaRoja] Jugador desconectado: %s"), *Exiting->GetName());
}

AActor* ARaidGameMode::ChoosePlayerStart_Implementation(AController* Player)
{
	// Seleccionar un PlayerStart aleatorio para distribuir los spawns de inserción
	TArray<AActor*> PlayerStarts;
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), APlayerStart::StaticClass(), PlayerStarts);

	if (PlayerStarts.Num() > 0)
	{
		const int32 RandomIndex = FMath::RandRange(0, PlayerStarts.Num() - 1);
		return PlayerStarts[RandomIndex];
	}

	return Super::ChoosePlayerStart_Implementation(Player);
}

// ============================================================
// CICLO DE VIDA DE LA INCURSION
// ============================================================

void ARaidGameMode::StartInsertionPhase()
{
	SetRaidPhase(ERaidPhase::Insercion);
	UE_LOG(LogTemp, Log, TEXT("[ZonaRoja] Fase de inserción iniciada"));
}

void ARaidGameMode::StartActivePhase()
{
	SetRaidPhase(ERaidPhase::Activa);

	// Iniciar el temporizador principal de la incursión
	GetWorldTimerManager().SetTimer(RaidTimerHandle, this,
		&ARaidGameMode::OnRaidTimerExpired, RaidDuration, false);

	// Tick cada segundo para actualizar el tiempo restante en GameState
	GetWorldTimerManager().SetTimer(RaidTickHandle, this,
		&ARaidGameMode::RaidTimerTick, 1.0f, true);

	UE_LOG(LogTemp, Log, TEXT("[ZonaRoja] Fase activa iniciada. Duración: %.0f segundos"), RaidDuration);
}

void ARaidGameMode::StartEvacuationPhase()
{
	SetRaidPhase(ERaidPhase::Evacuacion);

	// Restablecer el temporizador con la duración de evacuación
	GetWorldTimerManager().ClearTimer(RaidTimerHandle);
	GetWorldTimerManager().SetTimer(RaidTimerHandle, this,
		&ARaidGameMode::OnRaidTimerExpired, EvacuationDuration, false);

	if (ARaidGameState* GS = GetRaidGameState())
	{
		GS->RaidTimeRemaining = EvacuationDuration;
	}

	UE_LOG(LogTemp, Log, TEXT("[ZonaRoja] Fase de evacuación iniciada. Tiempo: %.0f segundos"), EvacuationDuration);
}

void ARaidGameMode::EndRaid()
{
	SetRaidPhase(ERaidPhase::Fin);

	// Detener todos los temporizadores de la incursión
	GetWorldTimerManager().ClearTimer(RaidTimerHandle);
	GetWorldTimerManager().ClearTimer(RaidTickHandle);

	UE_LOG(LogTemp, Log, TEXT("[ZonaRoja] Incursión finalizada"));

	// Dar tiempo para mostrar pantalla de resultados antes de volver al lobby
	FTimerHandle ReturnToLobbyHandle;
	GetWorldTimerManager().SetTimer(ReturnToLobbyHandle, [this]()
	{
		// Cargar el mapa del menú principal después de mostrar los resultados
		UGameplayStatics::OpenLevel(GetWorld(), FName(TEXT("/Game/Maps/MainMenu/MainMenu")), true);
	}, 15.0f, false);
}

void ARaidGameMode::SetRaidPhase(ERaidPhase NewPhase)
{
	if (CurrentPhase == NewPhase)
	{
		return; // No cambiar si ya estamos en la misma fase
	}

	const ERaidPhase OldPhase = CurrentPhase;
	CurrentPhase = NewPhase;

	// Actualizar el GameState replicado
	if (ARaidGameState* GS = GetRaidGameState())
	{
		GS->CurrentRaidPhase = NewPhase;
	}

	// Notificar a los suscriptores del cambio de fase
	OnRaidPhaseChanged.Broadcast(OldPhase, NewPhase);

	UE_LOG(LogTemp, Log, TEXT("[ZonaRoja] Cambio de fase: %d -> %d"),
		static_cast<int32>(OldPhase), static_cast<int32>(NewPhase));
}

// ============================================================
// GESTION DE JUGADORES
// ============================================================

void ARaidGameMode::RequestPlayerExtraction(ARaidPlayerController* PlayerController)
{
	if (!PlayerController)
	{
		return;
	}

	// Verificar que la incursión esté en una fase que permita extracción
	if (CurrentPhase != ERaidPhase::Activa && CurrentPhase != ERaidPhase::Evacuacion)
	{
		PlayerController->ClientShowNotification(
			FText::FromString(TEXT("No se puede extraer en esta fase")));
		return;
	}

	// La lógica de verificación de zona se maneja en el actor de zona de extracción
	// Aquí solo iniciamos el proceso de confirmación
	UE_LOG(LogTemp, Log, TEXT("[ZonaRoja] Solicitud de extracción de: %s"),
		*PlayerController->GetName());
}

void ARaidGameMode::ConfirmPlayerExtraction(ARaidPlayerController* PlayerController)
{
	if (!PlayerController)
	{
		return;
	}

	if (ARaidPlayerState* PS = PlayerController->GetPlayerState<ARaidPlayerState>())
	{
		// Marcar al jugador como extraído exitosamente
		PS->bHasExtracted = true;
		PS->bIsAlive = false;

		// Actualizar contadores en el GameState
		if (ARaidGameState* GS = GetRaidGameState())
		{
			GS->TotalPlayersExtracted++;
			GS->TotalPlayersAlive--;
		}

		// Notificar la extracción exitosa
		OnPlayerExtracted.Broadcast(PS);

		UE_LOG(LogTemp, Log, TEXT("[ZonaRoja] Jugador extraído: %s"), *PS->GetPlayerName());

		// Verificar si se debe terminar la incursión
		if (ShouldEndRaid())
		{
			EndRaid();
		}
	}
}

void ARaidGameMode::HandlePlayerDeath(ARaidPlayerController* VictimController,
	ARaidPlayerController* KillerController)
{
	if (!VictimController)
	{
		return;
	}

	ARaidPlayerState* VictimPS = VictimController->GetPlayerState<ARaidPlayerState>();
	ARaidPlayerState* KillerPS = KillerController ?
		KillerController->GetPlayerState<ARaidPlayerState>() : nullptr;

	if (VictimPS)
	{
		VictimPS->bIsAlive = false;
		VictimPS->DeathCount++;

		if (KillerPS)
		{
			KillerPS->KillCount++;
		}

		// Actualizar contadores globales
		if (ARaidGameState* GS = GetRaidGameState())
		{
			GS->TotalPlayersAlive = FMath::Max(0, GS->TotalPlayersAlive - 1);
		}

		// Notificar la muerte
		OnPlayerDied.Broadcast(VictimPS, KillerPS);

		UE_LOG(LogTemp, Log, TEXT("[ZonaRoja] Jugador muerto: %s | Asesino: %s"),
			*VictimPS->GetPlayerName(),
			KillerPS ? *KillerPS->GetPlayerName() : TEXT("Entorno"));
	}

	// Verificar condiciones de fin
	if (ShouldEndRaid())
	{
		EndRaid();
	}
}

bool ARaidGameMode::ShouldEndRaid() const
{
	if (CurrentPhase == ERaidPhase::Fin)
	{
		return false; // Ya terminó
	}

	const ARaidGameState* GS = GetRaidGameState();
	if (!GS)
	{
		return false;
	}

	// Terminar si no quedan jugadores vivos
	return GS->TotalPlayersAlive <= 0;
}

// ============================================================
// ZONAS DE EXTRACCION
// ============================================================

void ARaidGameMode::ActivateExtractionZone(FName ZoneID)
{
	if (ARaidGameState* GS = GetRaidGameState())
	{
		for (FExtractionZoneInfo& Zone : GS->ActiveExtractionZonesList)
		{
			if (Zone.ZoneID == ZoneID)
			{
				Zone.bIsActive = true;
				UE_LOG(LogTemp, Log, TEXT("[ZonaRoja] Zona de extracción activada: %s"),
					*ZoneID.ToString());
				return;
			}
		}
	}
}

void ARaidGameMode::DeactivateExtractionZone(FName ZoneID)
{
	if (ARaidGameState* GS = GetRaidGameState())
	{
		for (FExtractionZoneInfo& Zone : GS->ActiveExtractionZonesList)
		{
			if (Zone.ZoneID == ZoneID)
			{
				Zone.bIsActive = false;
				UE_LOG(LogTemp, Log, TEXT("[ZonaRoja] Zona de extracción desactivada: %s"),
					*ZoneID.ToString());
				return;
			}
		}
	}
}

void ARaidGameMode::RandomizeActiveExtractionZones()
{
	// Esta función se implementa completamente cuando las zonas
	// de extracción se colocan en el nivel como actores.
	// Por ahora registra la intención.
	UE_LOG(LogTemp, Log, TEXT("[ZonaRoja] Seleccionando %d zonas de extracción aleatorias"),
		ActiveExtractionZones);
}

// ============================================================
// TEMPORIZADORES INTERNOS
// ============================================================

void ARaidGameMode::RaidTimerTick()
{
	if (ARaidGameState* GS = GetRaidGameState())
	{
		// Decrementar el tiempo restante
		GS->RaidTimeRemaining = FMath::Max(0.0f, GS->RaidTimeRemaining - 1.0f);
	}
}

void ARaidGameMode::OnRaidTimerExpired()
{
	GetWorldTimerManager().ClearTimer(RaidTickHandle);

	if (CurrentPhase == ERaidPhase::Activa)
	{
		// Al expirar el tiempo activo, entrar en fase de evacuación
		StartEvacuationPhase();
	}
	else if (CurrentPhase == ERaidPhase::Evacuacion)
	{
		// Al expirar la evacuación, finalizar la incursión
		EndRaid();
	}
}

// ============================================================
// UTILIDADES
// ============================================================

ARaidGameState* ARaidGameMode::GetRaidGameState() const
{
	return GetGameState<ARaidGameState>();
}
