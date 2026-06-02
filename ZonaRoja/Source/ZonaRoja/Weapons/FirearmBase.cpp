// FirearmBase.cpp
// Implementación del arma de fuego base con hitscan y control de cadencia

#include "Weapons/FirearmBase.h"
#include "Components/HealthComponent.h"
#include "Kismet/GameplayStatics.h"
#include "NiagaraFunctionLibrary.h"
#include "Net/UnrealNetwork.h"
#include "Engine/World.h"
#include "DrawDebugHelpers.h"
#include "PhysicalMaterials/PhysicalMaterial.h"

AFirearmBase::AFirearmBase()
{
	PrimaryActorTick.bCanEverTick = true;
	PrimaryActorTick.TickInterval = 0.016f; // Tick a ~60fps para recuperación de dispersión

	// Cadencia predeterminada: 600 RPM (similar a un AK-103)
	RoundsPerMinute = 600.0f;
	BurstCount = 3;
	MaxHitscanRange = 10000.0f; // 100 metros

	// Dispersión predeterminada
	BaseSpreadAngle = 0.5f;
	MaxSpreadAngle = 6.0f;
	SpreadIncreasePerShot = 0.3f;
	SpreadRecoveryRate = 2.0f;
	ADSSpreadMultiplier = 0.25f;
	ProjectilesPerShot = 1;

	CurrentSpread = 0.0f;
	bIsAiming = false;
	bTriggerHeld = false;
	LastFireTime = -1.0f;
	BurstShotsRemaining = 0;

	// Configuración predeterminada: semiautomático y automático disponibles
	AvailableFireModes.Add(EFireMode::SemiAuto);
	AvailableFireModes.Add(EFireMode::FullAuto);
	CurrentFireMode = EFireMode::SemiAuto;
}

void AFirearmBase::BeginPlay()
{
	Super::BeginPlay();
}

void AFirearmBase::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	// Recuperar la dispersión gradualmente cuando no se está disparando
	if (!bTriggerHeld && CurrentSpread > BaseSpreadAngle)
	{
		CurrentSpread = FMath::Max(BaseSpreadAngle,
			CurrentSpread - SpreadRecoveryRate * DeltaTime);
	}
}

void AFirearmBase::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AFirearmBase, CurrentFireMode);
	DOREPLIFETIME(AFirearmBase, bIsAiming);
}

// ============================================================
// OVERRIDES DE DISPARO
// ============================================================

void AFirearmBase::Fire()
{
	if (!CanFire())
	{
		// Si no hay munición, intentar recarga automática
		if (!HasAmmoInMag() && HasReserveAmmo())
		{
			Reload();
		}
		else if (!HasAmmoInMag())
		{
			// Reproducir sonido de arma vacía
			if (EmptySound)
			{
				UGameplayStatics::PlaySoundAtLocation(GetWorld(), EmptySound, GetMuzzleLocation());
			}
		}
		return;
	}

	const float Now = GetWorld()->GetTimeSeconds();
	const float FireInterval = GetFireInterval();

	// Respetar la cadencia de disparo
	if (LastFireTime >= 0.0f && (Now - LastFireTime) < FireInterval)
	{
		return;
	}

	LastFireTime = Now;
	bTriggerHeld = true;

	if (HasAuthority())
	{
		// En el servidor: ejecutar hitscan directamente
		switch (CurrentFireMode)
		{
		case EFireMode::SemiAuto:
			PerformHitscanShot();
			break;
		case EFireMode::Burst:
			BurstShotsRemaining = BurstCount;
			PerformHitscanShot();
			BurstShotsRemaining--;
			if (BurstShotsRemaining > 0)
			{
				GetWorld()->GetTimerManager().SetTimer(FireTimerHandle, this,
					&AFirearmBase::AutoFireTick, FireInterval, true);
			}
			break;
		case EFireMode::FullAuto:
			PerformHitscanShot();
			if (!GetWorld()->GetTimerManager().IsTimerActive(FireTimerHandle))
			{
				GetWorld()->GetTimerManager().SetTimer(FireTimerHandle, this,
					&AFirearmBase::AutoFireTick, FireInterval, true);
			}
			break;
		}
	}
	else
	{
		// En el cliente: enviar al servidor para validación
		const FVector MuzzleLoc = GetMuzzleLocation();
		const FRotator MuzzleRot = GetMuzzleRotation();
		ServerFire(MuzzleLoc, MuzzleRot);

		// Configurar timer de fuego automático local (predicción del cliente)
		if (CurrentFireMode == EFireMode::FullAuto &&
			!GetWorld()->GetTimerManager().IsTimerActive(FireTimerHandle))
		{
			GetWorld()->GetTimerManager().SetTimer(FireTimerHandle, this,
				&AFirearmBase::AutoFireTick, FireInterval, true);
		}
	}
}

void AFirearmBase::StopFire()
{
	bTriggerHeld = false;

	// Detener el timer de fuego automático
	GetWorld()->GetTimerManager().ClearTimer(FireTimerHandle);
	BurstShotsRemaining = 0;
}

bool AFirearmBase::CanFire() const
{
	if (!Super::CanFire())
	{
		return false;
	}

	// Respetar la cadencia de disparo verificando el tiempo del último disparo
	// LastFireTime es -1 inicialmente (nunca disparado), siempre permitir el primer disparo
	if (LastFireTime < 0.0f)
	{
		return true;
	}

	const UWorld* World = GetWorld();
	if (!World)
	{
		return false;
	}

	const float Now = World->GetTimeSeconds();
	return (Now - LastFireTime >= GetFireInterval());
}

// ============================================================
// CONTROL DE MODO DE FUEGO
// ============================================================

void AFirearmBase::CycleFireMode()
{
	if (AvailableFireModes.Num() <= 1)
	{
		return; // Solo un modo disponible
	}

	// Encontrar el índice del modo actual y avanzar al siguiente
	int32 CurrentIndex = AvailableFireModes.IndexOfByKey(CurrentFireMode);
	CurrentIndex = (CurrentIndex + 1) % AvailableFireModes.Num();
	CurrentFireMode = AvailableFireModes[CurrentIndex];

	UE_LOG(LogTemp, Log, TEXT("[ZonaRoja] Modo de fuego cambiado: %d"), static_cast<int32>(CurrentFireMode));
}

void AFirearmBase::SetAiming(bool bNewAiming)
{
	bIsAiming = bNewAiming;

	// Al entrar en ADS, reducir la dispersión más rápido
	if (bIsAiming)
	{
		CurrentSpread = FMath::Min(CurrentSpread, BaseSpreadAngle * ADSSpreadMultiplier * 2.0f);
	}
}

float AFirearmBase::GetFireInterval() const
{
	// Convertir RPM a segundos entre disparos
	// Proteger contra división por cero
	return (RoundsPerMinute > 0.0f) ? (60.0f / RoundsPerMinute) : 0.1f;
}

// ============================================================
// HITSCAN
// ============================================================

void AFirearmBase::PerformHitscanShot()
{
	// Solo el servidor ejecuta el hitscan autoritativo
	if (!HasAuthority())
	{
		return;
	}

	// Disparar ProjectilesPerShot rayos (normalmente 1, más para escopetas)
	for (int32 i = 0; i < ProjectilesPerShot; ++i)
	{
		const FVector MuzzleLocation = GetMuzzleLocation();
		const FVector BaseDirection = GetMuzzleRotation().Vector();

		// Aplicar dispersión al ángulo del disparo
		const FVector ShotDirection = ApplySpreadToDirection(BaseDirection);
		const FVector TraceEnd = MuzzleLocation + (ShotDirection * MaxHitscanRange);

		FHitResult HitResult;
		FCollisionQueryParams QueryParams;
		QueryParams.AddIgnoredActor(this);
		QueryParams.AddIgnoredActor(GetInstigator());
		QueryParams.bReturnPhysicalMaterial = true; // Para efectos de superficie

		const bool bHit = GetWorld()->LineTraceSingleByChannel(HitResult, MuzzleLocation, TraceEnd,
			ECC_GameTraceChannel1, QueryParams); // Canal "Bullet"

#if WITH_EDITOR
		// Debug visual del disparo
		DrawDebugLine(GetWorld(), MuzzleLocation, bHit ? HitResult.ImpactPoint : TraceEnd,
			bHit ? FColor::Red : FColor::Yellow, false, 2.0f, 0, 1.5f);
#endif

		if (bHit)
		{
			ProcessHitResult(HitResult);
			MulticastPlayImpactEffects(HitResult);
		}
	}

	// Consumir munición y aumentar dispersión
	ConsumeAmmo();
	CurrentSpread = FMath::Min(MaxSpreadAngle, CurrentSpread + SpreadIncreasePerShot);

	// Efectos del disparo para todos los clientes
	MulticastPlayFireEffects(GetMuzzleLocation());
}

FVector AFirearmBase::ApplySpreadToDirection(const FVector& BaseDirection) const
{
	// Calcular la dispersión efectiva (reducida si apunta)
	const float EffectiveSpread = bIsAiming
		? CurrentSpread * ADSSpreadMultiplier
		: CurrentSpread;

	if (EffectiveSpread <= 0.0f)
	{
		return BaseDirection;
	}

	// Generar un desvío aleatorio dentro del cono de dispersión
	const float SpreadRadians = FMath::DegreesToRadians(EffectiveSpread);
	const float RandomAngle = FMath::FRandRange(0.0f, UE_TWO_PI);
	const float RandomRadius = FMath::FRandRange(0.0f, FMath::Tan(SpreadRadians));

	// Construir vectores perpendiculares al eje de disparo
	const FVector Right = FVector::CrossProduct(BaseDirection, FVector::UpVector).GetSafeNormal();
	const FVector Up = FVector::CrossProduct(Right, BaseDirection).GetSafeNormal();

	// Aplicar el desvío al vector de dirección
	const FVector SpreadOffset = (Right * FMath::Cos(RandomAngle) + Up * FMath::Sin(RandomAngle))
		* RandomRadius;

	return (BaseDirection + SpreadOffset).GetSafeNormal();
}

void AFirearmBase::ProcessHitResult(const FHitResult& HitResult)
{
	AActor* HitActor = HitResult.GetActor();
	if (!HitActor)
	{
		return;
	}

	// Intentar aplicar daño al componente de salud del actor impactado
	if (UHealthComponent* HealthComp = HitActor->FindComponentByClass<UHealthComponent>())
	{
		// Determinar la parte del cuerpo impactada basándose en el nombre del hueso
		EBodyPart HitBodyPart = EBodyPart::Chest; // Por defecto: pecho

		const FName BoneName = HitResult.BoneName;
		if (BoneName.ToString().Contains(TEXT("head")))
		{
			HitBodyPart = EBodyPart::Head;
		}
		else if (BoneName.ToString().Contains(TEXT("arm")) || BoneName.ToString().Contains(TEXT("hand")))
		{
			HitBodyPart = (BoneName.ToString().Contains(TEXT("l_")) || BoneName.ToString().Contains(TEXT("_l")))
				? EBodyPart::LeftArm : EBodyPart::RightArm;
		}
		else if (BoneName.ToString().Contains(TEXT("leg")) || BoneName.ToString().Contains(TEXT("thigh")))
		{
			HitBodyPart = (BoneName.ToString().Contains(TEXT("l_")) || BoneName.ToString().Contains(TEXT("_l")))
				? EBodyPart::LeftLeg : EBodyPart::RightLeg;
		}
		else if (BoneName.ToString().Contains(TEXT("spine")) || BoneName.ToString().Contains(TEXT("pelvis")))
		{
			HitBodyPart = EBodyPart::Stomach;
		}

		HealthComp->ApplyDamage(BaseDamage, HitBodyPart, EDamageType::Bullet,
			GetInstigator(), HitResult);
	}

	UE_LOG(LogTemp, Verbose, TEXT("[ZonaRoja] Impacto en: %s | Hueso: %s"),
		*HitActor->GetName(), *HitResult.BoneName.ToString());
}

void AFirearmBase::AutoFireTick()
{
	// Tick del timer de fuego automático
	if (!CanFire())
	{
		StopFire();
		return;
	}

	if (CurrentFireMode == EFireMode::Burst)
	{
		// Control de ráfaga
		if (BurstShotsRemaining > 0)
		{
			PerformHitscanShot();
			BurstShotsRemaining--;

			if (BurstShotsRemaining <= 0)
			{
				GetWorld()->GetTimerManager().ClearTimer(FireTimerHandle);
			}
		}
	}
	else if (CurrentFireMode == EFireMode::FullAuto && bTriggerHeld)
	{
		PerformHitscanShot();
	}
	else
	{
		StopFire();
	}
}

void AFirearmBase::StartSpreadRecovery()
{
	// La recuperación se hace en Tick; este método puede activar el tick si estaba deshabilitado
}

void AFirearmBase::SpreadRecoveryTick()
{
	// Este método ya no es necesario porque la recuperación está en Tick()
}

void AFirearmBase::MulticastPlayImpactEffects_Implementation(FHitResult HitResult)
{
	// Reproducir efectos visuales del impacto en la superficie
	// Los efectos específicos dependen del PhysicalMaterial de la superficie
	if (HitResult.PhysMaterial.IsValid())
	{
		// En la implementación completa: seleccionar Niagara y sonido según PhysMaterial
		// Para ahora: solo registrar el impacto
		UE_LOG(LogTemp, Verbose, TEXT("[ZonaRoja][VFX] Impacto en superficie: %s"),
			*HitResult.PhysMaterial->GetName());
	}
}
