// RaidPlayerState.cpp
// Implementación del estado del jugador para incursiones en ZonaRoja

#include "Core/RaidPlayerState.h"
#include "Net/UnrealNetwork.h"

ARaidPlayerState::ARaidPlayerState()
{
	// Estado inicial del jugador al entrar en una incursión
	bIsAlive = true;
	bHasExtracted = false;
	bIsInExtractionZone = false;
	CurrentHealth = 100.0f;
	MaxHealth = 100.0f;

	// Estadísticas iniciales en cero
	KillCount = 0;
	DeathCount = 0;
	TotalDamageDealt = 0.0f;
	TotalDamageTaken = 0.0f;
	ItemsLooted = 0;

	// Perfil por defecto
	PlayerLevel = 1;
	ExperienceGained = 0;
	FactionName = NAME_None;
}

// ============================================================
// REPLICACION
// ============================================================

void ARaidPlayerState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	// Estado vital: todos los clientes necesitan esto para el HUD de escuadra
	DOREPLIFETIME(ARaidPlayerState, bIsAlive);
	DOREPLIFETIME(ARaidPlayerState, bHasExtracted);
	DOREPLIFETIME(ARaidPlayerState, bIsInExtractionZone);
	DOREPLIFETIME(ARaidPlayerState, CurrentHealth);
	DOREPLIFETIME(ARaidPlayerState, MaxHealth);

	// Estadísticas de incursión
	DOREPLIFETIME(ARaidPlayerState, KillCount);
	DOREPLIFETIME(ARaidPlayerState, DeathCount);
	DOREPLIFETIME(ARaidPlayerState, TotalDamageDealt);
	DOREPLIFETIME(ARaidPlayerState, TotalDamageTaken);
	DOREPLIFETIME(ARaidPlayerState, ItemsLooted);

	// Información de perfil
	DOREPLIFETIME(ARaidPlayerState, PlayerLevel);
	DOREPLIFETIME(ARaidPlayerState, ExperienceGained);
	DOREPLIFETIME(ARaidPlayerState, FactionName);
}

// ============================================================
// FUNCIONES DE ESTADISTICAS
// ============================================================

void ARaidPlayerState::AddExperience(int32 Amount, const FString& Reason)
{
	if (Amount <= 0)
	{
		return;
	}

	ExperienceGained += Amount;

	UE_LOG(LogTemp, Log, TEXT("[ZonaRoja] %s ganó %d XP por: %s (Total: %d XP)"),
		*GetPlayerName(), Amount, *Reason, ExperienceGained);
}

void ARaidPlayerState::RegisterKill(const FString& VictimName)
{
	KillCount++;

	// Otorgar experiencia por baja
	AddExperience(100, FString::Printf(TEXT("Baja: %s"), *VictimName));

	UE_LOG(LogTemp, Log, TEXT("[ZonaRoja] %s eliminó a %s (Total bajas: %d)"),
		*GetPlayerName(), *VictimName, KillCount);
}

void ARaidPlayerState::RegisterDamageDealt(float DamageAmount)
{
	if (DamageAmount > 0.0f)
	{
		TotalDamageDealt += DamageAmount;
	}
}

void ARaidPlayerState::RegisterDamageTaken(float DamageAmount)
{
	if (DamageAmount > 0.0f)
	{
		TotalDamageTaken += DamageAmount;
	}
}

void ARaidPlayerState::ResetRaidStats()
{
	// Reiniciar estado vital
	bIsAlive = true;
	bHasExtracted = false;
	bIsInExtractionZone = false;
	CurrentHealth = MaxHealth;

	// Reiniciar estadísticas de la sesión (no las totales del perfil)
	KillCount = 0;
	TotalDamageDealt = 0.0f;
	TotalDamageTaken = 0.0f;
	ItemsLooted = 0;
	ExperienceGained = 0;

	UE_LOG(LogTemp, Log, TEXT("[ZonaRoja] Estadísticas reiniciadas para: %s"), *GetPlayerName());
}

void ARaidPlayerState::CopyProperties(APlayerState* PlayerState)
{
	Super::CopyProperties(PlayerState);

	// Copiar propiedades al hacer seamless travel entre mapas
	if (ARaidPlayerState* OtherState = Cast<ARaidPlayerState>(PlayerState))
	{
		OtherState->PlayerLevel = PlayerLevel;
		OtherState->FactionName = FactionName;
		// Las estadísticas de incursión no se copian (se reinician en cada raid)
	}
}
