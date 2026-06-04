// WeaponComponent.cpp
// Implementación del componente de gestión de armas para ZonaRoja

#include "Components/WeaponComponent.h"
#include "Weapons/WeaponBase.h"
#include "Weapons/FirearmBase.h"
#include "Net/UnrealNetwork.h"
#include "Engine/World.h"
#include "GameFramework/Character.h"

UWeaponComponent::UWeaponComponent()
{
	SetIsReplicatedByDefault(true);
	PrimaryComponentTick.bCanEverTick = false;

	// Nombres de sockets por defecto en el esqueleto del personaje
	WeaponSocketName = FName(TEXT("weapon_r"));   // Mano derecha
	BackSocketName = FName(TEXT("spine_03"));      // Espalda para arma guardada
	HolsterSocketName = FName(TEXT("thigh_r"));    // Muslo derecho para pistola

	ActiveWeaponSlot = EEquipmentSlot::PrimaryWeapon;
	bIsSwappingWeapon = false;
	bIsReloading = false;
}

void UWeaponComponent::BeginPlay()
{
	Super::BeginPlay();
}

void UWeaponComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(UWeaponComponent, CurrentWeapon);
	DOREPLIFETIME(UWeaponComponent, PrimaryWeapon);
	DOREPLIFETIME(UWeaponComponent, SecondaryWeapon);
	DOREPLIFETIME(UWeaponComponent, HolsterWeapon);
	DOREPLIFETIME(UWeaponComponent, ActiveWeaponSlot);
	DOREPLIFETIME(UWeaponComponent, bIsSwappingWeapon);
	DOREPLIFETIME(UWeaponComponent, bIsReloading);
}

// ============================================================
// SPAWN Y EQUIPAMIENTO
// ============================================================

AWeaponBase* UWeaponComponent::SpawnAndEquipWeapon(TSubclassOf<AWeaponBase> WeaponClass,
	EEquipmentSlot Slot)
{
	if (!GetOwner()->HasAuthority() || !WeaponClass)
	{
		return nullptr;
	}

	UWorld* World = GetWorld();
	if (!World)
	{
		return nullptr;
	}

	// Spawn del arma en la posición del personaje
	FActorSpawnParameters SpawnParams;
	SpawnParams.Owner = GetOwner();
	SpawnParams.Instigator = Cast<APawn>(GetOwner());
	SpawnParams.SpawnCollisionHandlingOverride =
		ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

	AWeaponBase* NewWeapon = World->SpawnActor<AWeaponBase>(WeaponClass,
		GetOwner()->GetActorLocation(), FRotator::ZeroRotator, SpawnParams);

	if (!NewWeapon)
	{
		return nullptr;
	}

	// Asignar a la ranura correspondiente
	switch (Slot)
	{
	case EEquipmentSlot::PrimaryWeapon:
		if (PrimaryWeapon) { PrimaryWeapon->Destroy(); }
		PrimaryWeapon = NewWeapon;
		break;
	case EEquipmentSlot::SecondaryWeapon:
		if (SecondaryWeapon) { SecondaryWeapon->Destroy(); }
		SecondaryWeapon = NewWeapon;
		break;
	case EEquipmentSlot::Holster:
		if (HolsterWeapon) { HolsterWeapon->Destroy(); }
		HolsterWeapon = NewWeapon;
		break;
	default:
		NewWeapon->Destroy();
		return nullptr;
	}

	// Adjuntar al socket de almacenamiento (guardada)
	AttachWeaponToSocket(NewWeapon, BackSocketName);

	OnWeaponEquipped.Broadcast(NewWeapon, Slot);

	UE_LOG(LogTemp, Log, TEXT("[ZonaRoja] Arma equipada en ranura %d: %s"),
		static_cast<int32>(Slot), *WeaponClass->GetName());

	return NewWeapon;
}

void UWeaponComponent::UnequipWeapon(EEquipmentSlot Slot)
{
	if (!GetOwner()->HasAuthority())
	{
		return;
	}

	// Obtener referencia al puntero en la ranura correspondiente
	AWeaponBase* WeaponToRemove = nullptr;
	switch (Slot)
	{
	case EEquipmentSlot::PrimaryWeapon:
		WeaponToRemove = PrimaryWeapon;
		PrimaryWeapon = nullptr;
		break;
	case EEquipmentSlot::SecondaryWeapon:
		WeaponToRemove = SecondaryWeapon;
		SecondaryWeapon = nullptr;
		break;
	case EEquipmentSlot::Holster:
		WeaponToRemove = HolsterWeapon;
		HolsterWeapon = nullptr;
		break;
	default: return;
	}

	if (WeaponToRemove)
	{
		if (WeaponToRemove == CurrentWeapon)
		{
			CurrentWeapon = nullptr;
		}
		WeaponToRemove->Destroy();
	}

	OnWeaponUnequipped.Broadcast(Slot);
}

// ============================================================
// CAMBIO DE ARMA
// ============================================================

void UWeaponComponent::SwitchToWeaponSlot(EEquipmentSlot Slot)
{
	if (!GetOwner()->HasAuthority() || bIsSwappingWeapon || bIsReloading)
	{
		return;
	}

	// No cambiar si ya estamos en esa ranura
	if (Slot == ActiveWeaponSlot)
	{
		return;
	}

	// Verificar que la ranura tiene un arma
	AWeaponBase* TargetWeapon = GetWeaponInSlot(Slot);
	if (!TargetWeapon)
	{
		return;
	}

	bIsSwappingWeapon = true;

	// Ocultar el arma actual durante el cambio
	if (CurrentWeapon)
	{
		CurrentWeapon->Unequip();
	}

	// Delay de 0.5 segundos para la animación de cambio de arma
	GetWorld()->GetTimerManager().SetTimer(WeaponSwitchTimer,
		FTimerDelegate::CreateUObject(this, &UWeaponComponent::FinishWeaponSwitch, Slot),
		0.5f, false);
}

void UWeaponComponent::FinishWeaponSwitch(EEquipmentSlot NewSlot)
{
	bIsSwappingWeapon = false;
	ActiveWeaponSlot = NewSlot;

	AWeaponBase* NewWeapon = GetWeaponInSlot(NewSlot);
	if (NewWeapon)
	{
		CurrentWeapon = NewWeapon;
		AttachWeaponToSocket(NewWeapon, WeaponSocketName);
		CurrentWeapon->Equip();

		OnWeaponEquipped.Broadcast(NewWeapon, NewSlot);
	}
}

// ============================================================
// DISPARO Y RECARGA
// ============================================================

void UWeaponComponent::StartFiring()
{
	if (!CanFire())
	{
		return;
	}

	CurrentWeapon->Fire();
}

void UWeaponComponent::StopFiring()
{
	if (CurrentWeapon)
	{
		CurrentWeapon->StopFire();
	}
}

void UWeaponComponent::StartReload()
{
	if (!CanReload())
	{
		return;
	}

	bIsReloading = true;

	// Detener cualquier disparo activo antes de recargar
	StopFiring();

	// Iniciar animación de recarga en el personaje
	OnReloadStarted.Broadcast(CurrentWeapon);

	// La duración de recarga depende del arma; usamos 2.5s por defecto
	const float ReloadDuration = 2.5f;
	GetWorld()->GetTimerManager().SetTimer(ReloadTimer, this,
		&UWeaponComponent::FinishReload, ReloadDuration, false);
}

void UWeaponComponent::CancelReload()
{
	if (bIsReloading)
	{
		bIsReloading = false;
		GetWorld()->GetTimerManager().ClearTimer(ReloadTimer);
	}
}

void UWeaponComponent::FinishReload()
{
	if (CurrentWeapon)
	{
		CurrentWeapon->Reload();
		OnReloadCompleted.Broadcast(CurrentWeapon);
	}

	bIsReloading = false;
}

void UWeaponComponent::ToggleFireMode()
{
	// La lógica de cambio de modo se delega al arma de fuego
	if (AFirearmBase* Firearm = Cast<AFirearmBase>(CurrentWeapon))
	{
		// La clase FirearmBase gestiona el ciclo de modos de fuego
		UE_LOG(LogTemp, Log, TEXT("[ZonaRoja] Cambiando modo de fuego"));
	}
}

// ============================================================
// CONSULTAS
// ============================================================

bool UWeaponComponent::CanFire() const
{
	return CurrentWeapon && !bIsSwappingWeapon && !bIsReloading;
}

bool UWeaponComponent::CanReload() const
{
	return CurrentWeapon && !bIsSwappingWeapon && !bIsReloading;
}

AWeaponBase* UWeaponComponent::GetWeaponInSlot(EEquipmentSlot Slot) const
{
	switch (Slot)
	{
	case EEquipmentSlot::PrimaryWeapon:   return PrimaryWeapon;
	case EEquipmentSlot::SecondaryWeapon: return SecondaryWeapon;
	case EEquipmentSlot::Holster:         return HolsterWeapon;
	default:                              return nullptr;
	}
}

// ============================================================
// UTILIDADES INTERNAS
// ============================================================

void UWeaponComponent::AttachWeaponToSocket(AWeaponBase* Weapon, FName SocketName)
{
	if (!Weapon)
	{
		return;
	}

	if (ACharacter* OwnerCharacter = Cast<ACharacter>(GetOwner()))
	{
		if (USkeletalMeshComponent* MeshComp = OwnerCharacter->GetMesh())
		{
			Weapon->AttachToComponent(MeshComp,
				FAttachmentTransformRules::SnapToTargetNotIncludingScale,
				SocketName);
		}
	}
}

void UWeaponComponent::OnRep_CurrentWeapon()
{
	// Los clientes actualizan sus efectos visuales cuando cambia el arma
	// La lógica de animación se gestiona en el AnimBP
}
