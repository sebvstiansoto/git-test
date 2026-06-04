// HealthComponent.cpp
// Implementación del componente de salud por partes del cuerpo para ZonaRoja

#include "Components/HealthComponent.h"
#include "Net/UnrealNetwork.h"
#include "Engine/World.h"

UHealthComponent::UHealthComponent()
{
	// Habilitar replicación del componente
	SetIsReplicatedByDefault(true);

	// Configuración por defecto del sistema de salud
	DefaultPartMaxHealth = 100.0f;

	// Multiplicadores de daño por zona del cuerpo
	HeadDamageMultiplier = 3.5f;      // Disparo en la cabeza: muy letal
	ChestDamageMultiplier = 1.0f;     // Pecho: daño estándar
	StomachDamageMultiplier = 1.2f;   // Estómago: ligeramente más dañino
	LimbDamageMultiplier = 0.6f;      // Extremidades: daño reducido

	bCanDie = true;
	bIsDead = false;
}

void UHealthComponent::BeginPlay()
{
	Super::BeginPlay();

	// Inicializar todas las partes del cuerpo al comenzar
	InitializeBodyParts();
}

void UHealthComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(UHealthComponent, BodyPartHealthData);
	DOREPLIFETIME(UHealthComponent, bIsDead);
}

// ============================================================
// INICIALIZACION
// ============================================================

void UHealthComponent::InitializeBodyParts()
{
	BodyPartHealthData.Empty();

	// Crear una entrada de salud para cada parte del cuerpo
	// Los multiplicadores de salud max reflejan la resistencia relativa de cada zona
	BodyPartHealthData.Add(FBodyPartHealth(EBodyPart::Head,     35.0f));   // Cabeza: frágil
	BodyPartHealthData.Add(FBodyPartHealth(EBodyPart::Chest,   DefaultPartMaxHealth)); // Pecho: estándar
	BodyPartHealthData.Add(FBodyPartHealth(EBodyPart::Stomach,  80.0f));   // Estómago
	BodyPartHealthData.Add(FBodyPartHealth(EBodyPart::LeftArm,  60.0f));   // Brazo izquierdo
	BodyPartHealthData.Add(FBodyPartHealth(EBodyPart::RightArm, 60.0f));   // Brazo derecho
	BodyPartHealthData.Add(FBodyPartHealth(EBodyPart::LeftLeg,  65.0f));   // Pierna izquierda
	BodyPartHealthData.Add(FBodyPartHealth(EBodyPart::RightLeg, 65.0f));   // Pierna derecha
}

// ============================================================
// FUNCIONES PRINCIPALES DE DAÑO
// ============================================================

void UHealthComponent::ApplyDamage(float BaseDamage, EBodyPart HitBodyPart, EDamageType DamageType,
	AActor* DamageInstigator, const FHitResult& HitResult)
{
	// Solo el servidor procesa el daño real
	if (!GetOwner()->HasAuthority())
	{
		return;
	}

	if (bIsDead)
	{
		return; // El actor ya está muerto, ignorar daño adicional
	}

	const int32 PartIndex = GetBodyPartIndex(HitBodyPart);
	if (PartIndex == INDEX_NONE)
	{
		return;
	}

	FBodyPartHealth& Part = BodyPartHealthData[PartIndex];

	// Calcular daño final con armadura y multiplicadores
	const float FinalDamage = CalculateFinalDamage(BaseDamage, Part, DamageType);

	// Aplicar el daño a la parte del cuerpo
	Part.CurrentHealth = FMath::Max(0.0f, Part.CurrentHealth - FinalDamage);

	// Verificar si el disparo genera sangrado (daño balístico alto)
	if (DamageType == EDamageType::Bullet && FinalDamage >= 15.0f)
	{
		const bool bStartsBleed = FMath::RandRange(0.0f, 1.0f) < 0.4f; // 40% de probabilidad
		if (bStartsBleed && !Part.bIsBleeding)
		{
			StartBleeding(FMath::FRandRange(1.0f, 3.5f), HitBodyPart);
		}
	}

	// Verificar fractura en extremidades por daño severo
	if ((HitBodyPart == EBodyPart::LeftLeg || HitBodyPart == EBodyPart::RightLeg ||
		 HitBodyPart == EBodyPart::LeftArm || HitBodyPart == EBodyPart::RightArm)
		&& FinalDamage >= 30.0f)
	{
		Part.bIsFractured = true;
	}

	// Notificar el daño a los suscriptores
	OnDamageReceived.Broadcast(FinalDamage, HitBodyPart, DamageType, DamageInstigator, HitResult);

	UE_LOG(LogTemp, Verbose, TEXT("[ZonaRoja] Daño aplicado: %.1f a parte %d (salud restante: %.1f)"),
		FinalDamage, static_cast<int32>(HitBodyPart), Part.CurrentHealth);

	// Comprobar si el actor debe morir
	// Muerte inmediata si la cabeza llega a cero, o si el pecho llega a cero
	const bool bIsLethalHit = (HitBodyPart == EBodyPart::Head && Part.IsDestroyed()) ||
		(HitBodyPart == EBodyPart::Chest && Part.IsDestroyed());

	// También verificar si la salud total es crítica
	if (bIsLethalHit || GetTotalHealth() <= 0.0f)
	{
		if (bCanDie)
		{
			ProcessDeath(DamageInstigator);
		}
	}
}

void UHealthComponent::HealBodyPart(float HealAmount, EBodyPart TargetPart)
{
	if (!GetOwner()->HasAuthority() || bIsDead)
	{
		return;
	}

	const int32 PartIndex = GetBodyPartIndex(TargetPart);
	if (PartIndex == INDEX_NONE)
	{
		return;
	}

	FBodyPartHealth& Part = BodyPartHealthData[PartIndex];

	// Restaurar salud sin exceder el máximo
	const float ActualHeal = FMath::Min(HealAmount, Part.MaxHealth - Part.CurrentHealth);
	Part.CurrentHealth = FMath::Min(Part.MaxHealth, Part.CurrentHealth + HealAmount);

	if (ActualHeal > 0.0f)
	{
		OnHealed.Broadcast(ActualHeal, TargetPart);
	}
}

void UHealthComponent::HealAll(float TotalHealAmount)
{
	if (!GetOwner()->HasAuthority() || bIsDead)
	{
		return;
	}

	// Distribuir la curación proporcionalmente entre las partes heridas
	for (FBodyPartHealth& Part : BodyPartHealthData)
	{
		if (Part.CurrentHealth < Part.MaxHealth)
		{
			const float Deficit = Part.MaxHealth - Part.CurrentHealth;
			const float HealForPart = FMath::Min(Deficit, TotalHealAmount / BodyPartHealthData.Num());
			Part.CurrentHealth += HealForPart;
		}
	}
}

void UHealthComponent::ApplyArmor(float ArmorValue, EBodyPart TargetPart)
{
	const int32 PartIndex = GetBodyPartIndex(TargetPart);
	if (PartIndex != INDEX_NONE)
	{
		BodyPartHealthData[PartIndex].ArmorValue += ArmorValue;
	}
}

// ============================================================
// SISTEMA DE SANGRADO
// ============================================================

void UHealthComponent::StartBleeding(float BleedRate, EBodyPart AffectedPart)
{
	const int32 PartIndex = GetBodyPartIndex(AffectedPart);
	if (PartIndex == INDEX_NONE)
	{
		return;
	}

	FBodyPartHealth& Part = BodyPartHealthData[PartIndex];
	if (!Part.bIsBleeding)
	{
		Part.bIsBleeding = true;
		Part.BleedRate = BleedRate;

		// Iniciar el timer de sangrado si no está corriendo
		if (!GetWorld()->GetTimerManager().IsTimerActive(BleedingTimerHandle))
		{
			GetWorld()->GetTimerManager().SetTimer(BleedingTimerHandle, this,
				&UHealthComponent::BleedingTick, 1.0f, true);
		}

		OnBleedingStateChanged.Broadcast(AffectedPart, true);
	}
}

void UHealthComponent::StopBleeding(EBodyPart AffectedPart)
{
	const int32 PartIndex = GetBodyPartIndex(AffectedPart);
	if (PartIndex == INDEX_NONE)
	{
		return;
	}

	FBodyPartHealth& Part = BodyPartHealthData[PartIndex];
	if (Part.bIsBleeding)
	{
		Part.bIsBleeding = false;
		Part.BleedRate = 0.0f;

		// Verificar si alguna otra parte sigue sangrando
		if (!IsAnyPartBleeding())
		{
			GetWorld()->GetTimerManager().ClearTimer(BleedingTimerHandle);
		}

		OnBleedingStateChanged.Broadcast(AffectedPart, false);
	}
}

void UHealthComponent::BleedingTick()
{
	if (!GetOwner()->HasAuthority() || bIsDead)
	{
		GetWorld()->GetTimerManager().ClearTimer(BleedingTimerHandle);
		return;
	}

	// Aplicar daño de sangrado a cada parte que esté sangrando
	for (FBodyPartHealth& Part : BodyPartHealthData)
	{
		if (Part.bIsBleeding && Part.BleedRate > 0.0f)
		{
			Part.CurrentHealth = FMath::Max(0.0f, Part.CurrentHealth - Part.BleedRate);

			// Si la parte queda destruida por sangrado, verificar muerte
			if (Part.IsDestroyed() && bCanDie)
			{
				ProcessDeath(nullptr);
				return;
			}
		}
	}
}

// ============================================================
// CONSULTAS
// ============================================================

float UHealthComponent::GetBodyPartHealth(EBodyPart BodyPart) const
{
	const int32 PartIndex = GetBodyPartIndex(BodyPart);
	if (PartIndex != INDEX_NONE)
	{
		return BodyPartHealthData[PartIndex].CurrentHealth;
	}
	return 0.0f;
}

float UHealthComponent::GetTotalHealth() const
{
	float Total = 0.0f;
	for (const FBodyPartHealth& Part : BodyPartHealthData)
	{
		Total += Part.CurrentHealth;
	}
	return Total;
}

float UHealthComponent::GetTotalMaxHealth() const
{
	float Total = 0.0f;
	for (const FBodyPartHealth& Part : BodyPartHealthData)
	{
		Total += Part.MaxHealth;
	}
	return Total;
}

bool UHealthComponent::IsAnyPartBleeding() const
{
	for (const FBodyPartHealth& Part : BodyPartHealthData)
	{
		if (Part.bIsBleeding)
		{
			return true;
		}
	}
	return false;
}

// ============================================================
// CALCULOS INTERNOS
// ============================================================

float UHealthComponent::CalculateFinalDamage(float RawDamage, const FBodyPartHealth& Part,
	EDamageType DmgType) const
{
	float FinalDamage = RawDamage;

	// Aplicar modificador por zona del cuerpo
	FinalDamage *= GetBodyPartDamageMultiplier(Part.BodyPart);

	// Calcular penetración de armadura
	// Fórmula: daño efectivo = max(0, daño - armadura * 0.5)
	// La armadura no bloquea completamente, solo reduce el daño
	if (Part.ArmorValue > 0.0f && DmgType == EDamageType::Bullet)
	{
		const float ArmorReduction = Part.ArmorValue * 0.5f;
		FinalDamage = FMath::Max(FinalDamage - ArmorReduction, FinalDamage * 0.1f);
		// Desgastar la armadura con cada impacto
		// (La armadura es una referencia const aquí; el desgaste se gestiona externamente)
	}

	// Los explosivos ignoran parcialmente la armadura
	if (DmgType == EDamageType::Explosive)
	{
		FinalDamage = RawDamage * GetBodyPartDamageMultiplier(Part.BodyPart) * 0.8f;
	}

	return FMath::Max(0.0f, FinalDamage);
}

float UHealthComponent::GetBodyPartDamageMultiplier(EBodyPart BodyPart) const
{
	switch (BodyPart)
	{
	case EBodyPart::Head:      return HeadDamageMultiplier;
	case EBodyPart::Chest:     return ChestDamageMultiplier;
	case EBodyPart::Stomach:   return StomachDamageMultiplier;
	case EBodyPart::LeftArm:
	case EBodyPart::RightArm:
	case EBodyPart::LeftLeg:
	case EBodyPart::RightLeg:  return LimbDamageMultiplier;
	default:                   return 1.0f;
	}
}

int32 UHealthComponent::GetBodyPartIndex(EBodyPart BodyPart) const
{
	for (int32 i = 0; i < BodyPartHealthData.Num(); ++i)
	{
		if (BodyPartHealthData[i].BodyPart == BodyPart)
		{
			return i;
		}
	}
	return INDEX_NONE;
}

void UHealthComponent::ProcessDeath(AActor* Killer)
{
	if (bIsDead)
	{
		return; // Evitar procesar la muerte múltiples veces
	}

	bIsDead = true;

	// Detener el timer de sangrado
	GetWorld()->GetTimerManager().ClearTimer(BleedingTimerHandle);

	// Notificar a todos los suscriptores de la muerte
	OnDeath.Broadcast(GetOwner(), Killer);

	UE_LOG(LogTemp, Log, TEXT("[ZonaRoja] Actor muerto: %s | Asesino: %s"),
		*GetOwner()->GetName(),
		Killer ? *Killer->GetName() : TEXT("Entorno/Sangrado"));
}

void UHealthComponent::OnRep_BodyPartHealthData()
{
	// Los clientes actualizan sus efectos visuales cuando cambian los datos de salud
	// La lógica visual (indicadores de daño, etc.) se maneja en el Blueprint del personaje
}
