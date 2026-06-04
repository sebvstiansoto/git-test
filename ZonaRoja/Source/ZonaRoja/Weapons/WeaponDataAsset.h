// WeaponDataAsset.h
// Asset de datos de configuración de armas (se asigna en el Blueprint del arma)

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "Data/ZRTypes.h"
#include "WeaponDataAsset.generated.h"

/**
 * UWeaponDataAsset
 * Asset de datos que centraliza la configuración de un arma específica.
 * Se crea una instancia por tipo de arma (AK-103, M4A1, etc.) y se asigna
 * en el Blueprint del arma correspondiente.
 */
UCLASS(BlueprintType)
class ZONAROJA_API UWeaponDataAsset : public UDataAsset
{
	GENERATED_BODY()

public:
	/** Nombre del arma para mostrar en el HUD e inventario */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Arma|Info")
	FText WeaponName;

	/** Descripción corta para el tooltip del inventario */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Arma|Info")
	FText WeaponDescription;

	/** Categoría del arma (Rifle, Pistola, Escopeta, etc.) */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Arma|Info")
	EWeaponCategory WeaponCategory = EWeaponCategory::Rifle;

	/** Tipo de munición que usa esta arma */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Arma|Munición")
	EAmmoType AmmoType = EAmmoType::E556x45;

	/** Capacidad máxima del cargador estándar */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Arma|Munición")
	int32 MagCapacity = 30;

	/** Daño base por impacto en el pecho */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Arma|Estadísticas")
	float BaseDamage = 45.0f;

	/** Velocidad del proyectil en m/s */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Arma|Estadísticas")
	float MuzzleVelocity = 900.0f;

	/** Penetración de armadura (0-100) */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Arma|Estadísticas")
	float ArmorPenetration = 40.0f;

	/** Cadencia de disparo en RPM */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Arma|Estadísticas")
	float RoundsPerMinute = 600.0f;

	/** Icono del arma para el inventario */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Arma|Visual")
	TObjectPtr<UTexture2D> WeaponIcon;
};
