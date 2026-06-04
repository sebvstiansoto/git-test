// RaidGameState.cpp
// Implementación del estado de juego para incursiones en ZonaRoja

#include "Core/RaidGameState.h"
#include "Net/UnrealNetwork.h"

ARaidGameState::ARaidGameState()
{
	// Inicializar valores predeterminados
	RaidTimeRemaining = 2700.0f;   // 45 minutos
	CurrentRaidPhase = ERaidPhase::Insercion;
	TotalPlayersAlive = 0;
	TotalPlayersExtracted = 0;
}

// ============================================================
// REPLICACION
// ============================================================

void ARaidGameState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	// Registrar todas las propiedades replicadas para que UE las sincronice
	DOREPLIFETIME(ARaidGameState, RaidTimeRemaining);
	DOREPLIFETIME(ARaidGameState, CurrentRaidPhase);
	DOREPLIFETIME(ARaidGameState, TotalPlayersAlive);
	DOREPLIFETIME(ARaidGameState, TotalPlayersExtracted);
	DOREPLIFETIME(ARaidGameState, ActiveExtractionZonesList);
}

// ============================================================
// CALLBACKS DE REPLICACION
// ============================================================

void ARaidGameState::OnRep_RaidTimeRemaining()
{
	// El HUD puede suscribirse a este evento para actualizar el temporizador visual
	// La lógica de UI se maneja en el PlayerController o HUD
}

void ARaidGameState::OnRep_CurrentRaidPhase()
{
	// Notificar a los sistemas del cliente sobre el cambio de fase
	// Los Blueprints pueden sobreescribir esto para actualizar la UI
	UE_LOG(LogTemp, Verbose, TEXT("[ZonaRoja][Cliente] Fase de incursión actualizada: %d"),
		static_cast<int32>(CurrentRaidPhase));
}

void ARaidGameState::OnRep_ActiveExtractionZonesList()
{
	// Actualizar los marcadores del minimapa cuando las zonas cambien
	UE_LOG(LogTemp, Verbose, TEXT("[ZonaRoja][Cliente] Zonas de extracción actualizadas: %d zonas"),
		ActiveExtractionZonesList.Num());
}

// ============================================================
// FUNCIONES DE UTILIDAD
// ============================================================

FText ARaidGameState::GetFormattedTimeRemaining() const
{
	// Convertir segundos a formato MM:SS para el HUD
	const int32 TotalSeconds = FMath::Max(0, static_cast<int32>(RaidTimeRemaining));
	const int32 Minutes = TotalSeconds / 60;
	const int32 Seconds = TotalSeconds % 60;

	return FText::FromString(FString::Printf(TEXT("%02d:%02d"), Minutes, Seconds));
}

FText ARaidGameState::GetCurrentPhaseDisplayName() const
{
	// Devolver el nombre legible de la fase actual para el HUD
	switch (CurrentRaidPhase)
	{
	case ERaidPhase::Insercion:
		return FText::FromString(TEXT("INSERCIÓN"));
	case ERaidPhase::Activa:
		return FText::FromString(TEXT("INCURSIÓN ACTIVA"));
	case ERaidPhase::Evacuacion:
		return FText::FromString(TEXT("EVACUACIÓN"));
	case ERaidPhase::Fin:
		return FText::FromString(TEXT("FIN DE INCURSIÓN"));
	default:
		return FText::FromString(TEXT("DESCONOCIDO"));
	}
}

bool ARaidGameState::GetExtractionZoneByID(FName ZoneID, FExtractionZoneInfo& OutZone) const
{
	for (const FExtractionZoneInfo& Zone : ActiveExtractionZonesList)
	{
		if (Zone.ZoneID == ZoneID)
		{
			OutZone = Zone;
			return true;
		}
	}
	return false;
}
