// StaminaComponent.cpp
// Implementación del componente de stamina para ZonaRoja

#include "Components/StaminaComponent.h"
#include "Net/UnrealNetwork.h"

UStaminaComponent::UStaminaComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
	SetIsReplicatedByDefault(true);

	// Valores de stamina por defecto para un combatiente con equipo táctico
	MaxStamina = 100.0f;
	CurrentStamina = 100.0f;
	StaminaRegenRate = 15.0f;      // 15 stamina/seg en reposo
	WalkingRegenRate = 8.0f;       // 8 stamina/seg caminando
	SprintStaminaDrain = 20.0f;    // 20 stamina/seg esprintando
	JumpStaminaCost = 15.0f;       // Costo fijo por salto
	RegenDelay = 2.0f;             // 2 segundos antes de regenerar
	MinStaminaToSprint = 10.0f;    // Mínimo para iniciar sprint

	bIsSprinting = false;
	bIsExhausted = false;
	TimeSinceLastDrain = 0.0f;
}

void UStaminaComponent::BeginPlay()
{
	Super::BeginPlay();
	CurrentStamina = MaxStamina;
}

void UStaminaComponent::TickComponent(float DeltaTime, ELevelTick TickType,
	FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	// Solo el servidor procesa la stamina; los clientes reciben la replicación
	if (!GetOwner()->HasAuthority())
	{
		return;
	}

	TimeSinceLastDrain += DeltaTime;

	if (bIsSprinting)
	{
		UpdateSprintDrain(DeltaTime);
	}
	else if (TimeSinceLastDrain >= RegenDelay)
	{
		UpdateStaminaRegen(DeltaTime);
	}
}

void UStaminaComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(UStaminaComponent, CurrentStamina);
	DOREPLIFETIME(UStaminaComponent, bIsSprinting);
	DOREPLIFETIME(UStaminaComponent, bIsExhausted);
}

// ============================================================
// FUNCIONES PRINCIPALES
// ============================================================

bool UStaminaComponent::StartSprinting()
{
	if (!CanSprint())
	{
		return false;
	}

	bIsSprinting = true;
	TimeSinceLastDrain = 0.0f;
	return true;
}

void UStaminaComponent::StopSprinting()
{
	if (bIsSprinting)
	{
		bIsSprinting = false;
		TimeSinceLastDrain = 0.0f; // Reiniciar el delay de regeneración
	}
}

bool UStaminaComponent::ConsumeJumpStamina()
{
	if (!CanJump())
	{
		return false;
	}

	ConsumeStamina(JumpStaminaCost);
	return true;
}

void UStaminaComponent::ConsumeStamina(float Amount)
{
	if (!GetOwner()->HasAuthority())
	{
		return;
	}

	const float PreviousStamina = CurrentStamina;
	CurrentStamina = FMath::Max(0.0f, CurrentStamina - Amount);
	TimeSinceLastDrain = 0.0f;

	// Notificar el cambio
	OnStaminaChanged.Broadcast(CurrentStamina, MaxStamina);

	// Verificar si se agotó la stamina
	if (CurrentStamina <= 0.0f && PreviousStamina > 0.0f)
	{
		bIsExhausted = true;
		bIsSprinting = false;
		OnStaminaDepleted.Broadcast();
	}
}

// ============================================================
// CONSULTAS
// ============================================================

float UStaminaComponent::GetStaminaPercent() const
{
	return (MaxStamina > 0.0f) ? (CurrentStamina / MaxStamina) : 0.0f;
}

bool UStaminaComponent::CanSprint() const
{
	return !bIsExhausted && CurrentStamina >= MinStaminaToSprint;
}

bool UStaminaComponent::CanJump() const
{
	return CurrentStamina >= JumpStaminaCost && !bIsExhausted;
}

float UStaminaComponent::GetAimingPenaltyMultiplier() const
{
	// Cuanto más cansado, peor la puntería
	// Rango: 1.0 (descansado) a 3.0 (agotado por sprint)
	if (bIsExhausted)
	{
		return 3.0f;
	}

	const float StaminaPercent = GetStaminaPercent();

	if (bIsSprinting)
	{
		// Puntería muy deteriorada durante el sprint
		return FMath::Lerp(2.0f, 3.0f, 1.0f - StaminaPercent);
	}

	// Penalización gradual al bajar la stamina
	return FMath::Lerp(1.0f, 2.0f, 1.0f - StaminaPercent);
}

// ============================================================
// FUNCIONES INTERNAS
// ============================================================

void UStaminaComponent::UpdateStaminaRegen(float DeltaTime)
{
	if (CurrentStamina >= MaxStamina)
	{
		return; // Ya al máximo
	}

	const float RegenRate = StaminaRegenRate;
	const float OldStamina = CurrentStamina;
	CurrentStamina = FMath::Min(MaxStamina, CurrentStamina + RegenRate * DeltaTime);

	// Quitar estado de agotamiento cuando se recupera el 30% de la stamina
	if (bIsExhausted && CurrentStamina >= MaxStamina * 0.3f)
	{
		bIsExhausted = false;
	}

	if (!FMath::IsNearlyEqual(OldStamina, CurrentStamina, 0.1f))
	{
		OnStaminaChanged.Broadcast(CurrentStamina, MaxStamina);
	}
}

void UStaminaComponent::UpdateSprintDrain(float DeltaTime)
{
	ConsumeStamina(SprintStaminaDrain * DeltaTime);

	// Detener el sprint automáticamente si se acaba la stamina
	if (CurrentStamina <= 0.0f)
	{
		StopSprinting();
	}
}

void UStaminaComponent::OnRep_CurrentStamina()
{
	// Notificar a la UI del cliente cuando la stamina se actualiza
	OnStaminaChanged.Broadcast(CurrentStamina, MaxStamina);
}
