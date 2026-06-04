// WeaponBase.cpp
// Implementación de la clase base de armas para ZonaRoja

#include "Weapons/WeaponBase.h"
#include "Components/SkeletalMeshComponent.h"
#include "NiagaraFunctionLibrary.h"
#include "Kismet/GameplayStatics.h"
#include "Net/UnrealNetwork.h"

AWeaponBase::AWeaponBase()
{
	// El arma no necesita tick propio; el firing timer se gestiona en subclases
	PrimaryActorTick.bCanEverTick = false;
	bReplicates = true;
	SetReplicatingMovement(true);

	// Crear componente de mallado del arma como raíz
	WeaponMesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("WeaponMesh"));
	SetRootComponent(WeaponMesh);
	WeaponMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	WeaponMesh->SetCastShadow(true);

	// Valores predeterminados de munición y estadísticas
	MagCapacity = 30;
	CurrentAmmoInMag = MagCapacity;
	TotalAmmo = 90;          // 3 cargadores de reserva
	BaseDamage = 45.0f;
	MuzzleVelocity = 715.0f; // Velocidad estándar para 7.62x39mm (m/s)
	ArmorPenetration = 40.0f;

	bIsEquipped = false;
	bIsReloading = false;

	MuzzleSocketName = FName(TEXT("Muzzle"));

	// Categoría y munición por defecto
	WeaponCategory = EWeaponCategory::Rifle;
	AmmoType = EAmmoType::E762x39;
}

void AWeaponBase::BeginPlay()
{
	Super::BeginPlay();
}

void AWeaponBase::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AWeaponBase, CurrentAmmoInMag);
	DOREPLIFETIME(AWeaponBase, TotalAmmo);
	DOREPLIFETIME(AWeaponBase, bIsEquipped);
	DOREPLIFETIME(AWeaponBase, bIsReloading);
}

// ============================================================
// FUNCIONES VIRTUALES PRINCIPALES
// ============================================================

void AWeaponBase::Fire()
{
	// Las subclases implementan el mecanismo de disparo específico
	// Esta implementación base maneja la lógica común
	if (!CanFire())
	{
		return;
	}

	// Enviar al servidor para validación
	if (!HasAuthority())
	{
		const FVector MuzzleLoc = GetMuzzleLocation();
		const FRotator MuzzleRot = GetMuzzleRotation();
		ServerFire(MuzzleLoc, MuzzleRot);
	}
}

void AWeaponBase::StopFire()
{
	// Las subclases con fuego automático cancelan sus timers aquí
}

void AWeaponBase::Reload()
{
	if (!HasAuthority() || bIsReloading)
	{
		return;
	}

	if (CurrentAmmoInMag >= MagCapacity || TotalAmmo <= 0)
	{
		return; // El cargador ya está lleno o no hay munición de reserva
	}

	ProcessReload();
}

void AWeaponBase::Equip()
{
	bIsEquipped = true;
	WeaponMesh->SetVisibility(true);

	UE_LOG(LogTemp, Log, TEXT("[ZonaRoja] Arma equipada: %s"), *GetName());
}

void AWeaponBase::Unequip()
{
	bIsEquipped = false;

	// Ocultar el mallado del arma cuando está guardada en la espalda
	// El socket de la espalda sigue teniendo el arma adjunta, solo invisible
	// En la implementación final: mostrar el arma en el socket de la espalda
	UE_LOG(LogTemp, Log, TEXT("[ZonaRoja] Arma desequipada: %s"), *GetName());
}

bool AWeaponBase::CanFire() const
{
	return bIsEquipped && HasAmmoInMag() && !bIsReloading;
}

FVector AWeaponBase::GetMuzzleLocation() const
{
	if (WeaponMesh && WeaponMesh->DoesSocketExist(MuzzleSocketName))
	{
		return WeaponMesh->GetSocketLocation(MuzzleSocketName);
	}
	return GetActorLocation();
}

FRotator AWeaponBase::GetMuzzleRotation() const
{
	if (WeaponMesh && WeaponMesh->DoesSocketExist(MuzzleSocketName))
	{
		return WeaponMesh->GetSocketRotation(MuzzleSocketName);
	}
	return GetActorRotation();
}

// ============================================================
// RPC SERVIDOR
// ============================================================

void AWeaponBase::ServerFire_Implementation(FVector MuzzleLocation, FRotator ShotRotation)
{
	// El servidor valida el disparo y aplica el daño
	// La validación completa incluye anti-cheat (comparar con posición del cuerpo)
	if (!CanFire())
	{
		return;
	}

	ConsumeAmmo();
	MulticastPlayFireEffects(MuzzleLocation);

	// Notificar a los observadores del disparo
	OnWeaponFired.Broadcast(this, MuzzleLocation);

	UE_LOG(LogTemp, Verbose, TEXT("[ZonaRoja][Servidor] Disparo validado de: %s | Munición: %d/%d"),
		*GetName(), CurrentAmmoInMag, TotalAmmo);
}

// ============================================================
// MULTICAST DE EFECTOS
// ============================================================

void AWeaponBase::MulticastPlayFireEffects_Implementation(FVector MuzzleLocation)
{
	// Reproducir fogonazo en todos los clientes
	if (MuzzleFlashEffect)
	{
		UNiagaraFunctionLibrary::SpawnSystemAtLocation(GetWorld(), MuzzleFlashEffect,
			MuzzleLocation, GetMuzzleRotation());
	}

	// Reproducir sonido del disparo
	if (FireSound)
	{
		UGameplayStatics::PlaySoundAtLocation(GetWorld(), FireSound, MuzzleLocation);
	}
}

// ============================================================
// UTILIDADES INTERNAS
// ============================================================

void AWeaponBase::ConsumeAmmo()
{
	if (CurrentAmmoInMag > 0)
	{
		CurrentAmmoInMag--;

		// Notificar si el cargador queda vacío
		if (CurrentAmmoInMag == 0)
		{
			OnWeaponEmpty.Broadcast(this);

			if (EmptySound)
			{
				UGameplayStatics::PlaySoundAtLocation(GetWorld(), EmptySound, GetMuzzleLocation());
			}
		}
	}
}

void AWeaponBase::ProcessReload()
{
	// Calcular cuánta munición necesitamos para llenar el cargador
	const int32 AmmoNeeded = MagCapacity - CurrentAmmoInMag;
	const int32 AmmoToAdd = FMath::Min(AmmoNeeded, TotalAmmo);

	CurrentAmmoInMag += AmmoToAdd;
	TotalAmmo -= AmmoToAdd;

	bIsReloading = false;
	OnWeaponReloadComplete.Broadcast(this);

	UE_LOG(LogTemp, Log, TEXT("[ZonaRoja] Recarga completada: %s | Cargador: %d/%d | Reserva: %d"),
		*GetName(), CurrentAmmoInMag, MagCapacity, TotalAmmo);
}
