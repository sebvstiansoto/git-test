// ZRTypes.h
// Cabecera de tipos compartidos para el proyecto ZonaRoja
// Define todos los enumerados y estructuras usados en todo el proyecto

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "ZRTypes.generated.h"

// ============================================================
// ENUMERADOS DE FASE DE INCURSION
// ============================================================

/** Fases del ciclo de vida de una incursión */
UENUM(BlueprintType)
enum class ERaidPhase : uint8
{
	Insercion	UMETA(DisplayName = "Inserción"),		// Jugadores entrando al mapa
	Activa		UMETA(DisplayName = "Activa"),			// Fase principal de juego
	Evacuacion	UMETA(DisplayName = "Evacuación"),		// Tiempo de extracción reduciéndose
	Fin			UMETA(DisplayName = "Fin")				// Incursión terminada
};

// ============================================================
// ENUMERADOS DE PARTES DEL CUERPO Y DAÑO
// ============================================================

/** Partes del cuerpo para el sistema de daño localizado */
UENUM(BlueprintType)
enum class EBodyPart : uint8
{
	Head		UMETA(DisplayName = "Cabeza"),
	Chest		UMETA(DisplayName = "Pecho"),
	Stomach		UMETA(DisplayName = "Estómago"),
	LeftArm		UMETA(DisplayName = "Brazo Izquierdo"),
	RightArm	UMETA(DisplayName = "Brazo Derecho"),
	LeftLeg		UMETA(DisplayName = "Pierna Izquierda"),
	RightLeg	UMETA(DisplayName = "Pierna Derecha")
};

/** Tipos de daño para calcular modificadores y efectos */
UENUM(BlueprintType)
enum class EDamageType : uint8
{
	Bullet		UMETA(DisplayName = "Bala"),			// Daño balístico estándar
	Explosive	UMETA(DisplayName = "Explosivo"),		// Granada, mina, RPG
	Melee		UMETA(DisplayName = "Cuerpo a Cuerpo"),	// Cuchillo, golpe
	Fire		UMETA(DisplayName = "Fuego"),			// Daño por quemadura
	Bleed		UMETA(DisplayName = "Sangrado"),		// Daño por tiempo (sangrado)
	Fall		UMETA(DisplayName = "Caída"),			// Daño por caída
	Radiation	UMETA(DisplayName = "Radiación"),		// Zona de anomalía
	Generic		UMETA(DisplayName = "Genérico")			// Daño sin tipo específico
};

// ============================================================
// ENUMERADOS DE ARMAS
// ============================================================

/** Categorías de armas disponibles en el juego */
UENUM(BlueprintType)
enum class EWeaponCategory : uint8
{
	Rifle		UMETA(DisplayName = "Rifle de Asalto"),
	SMG			UMETA(DisplayName = "Subfusil"),
	Sniper		UMETA(DisplayName = "Francotirador"),
	Shotgun		UMETA(DisplayName = "Escopeta"),
	LMG			UMETA(DisplayName = "Ametralladora Ligera"),
	Pistol		UMETA(DisplayName = "Pistola")
};

/** Tipos de munición disponibles en el mundo del juego */
UENUM(BlueprintType)
enum class EAmmoType : uint8
{
	E762x39		UMETA(DisplayName = "7.62x39mm"),		// AK-103, SKS
	E545x39		UMETA(DisplayName = "5.45x39mm"),		// AK-74, RPK-74
	E556x45		UMETA(DisplayName = "5.56x45mm"),		// M4A1, HK416
	E9mm		UMETA(DisplayName = "9x19mm"),			// MP5, G17, MP9
	E762x54R	UMETA(DisplayName = "7.62x54mmR"),		// Mosin, SVD, PKM
	E12gauge	UMETA(DisplayName = "12 Gauge")			// M870, Saiga-12
};

/** Ranuras de accesorios disponibles en las armas */
UENUM(BlueprintType)
enum class EAttachmentSlot : uint8
{
	Optic		UMETA(DisplayName = "Óptica"),			// Miras y puntos rojos
	Muzzle		UMETA(DisplayName = "Boca del Cañón"),	// Silenciadores, compensadores
	Grip		UMETA(DisplayName = "Empuñadura"),		// Empuñaduras frontales
	Stock		UMETA(DisplayName = "Culata"),			// Culatas plegables o fijas
	Magazine	UMETA(DisplayName = "Cargador"),		// Cargadores extendidos
	Tactical	UMETA(DisplayName = "Táctico")			// Linternas, láseres
};

// ============================================================
// ENUMERADOS DE ITEMS Y EQUIPAMIENTO
// ============================================================

/** Niveles de rareza de los objetos del juego */
UENUM(BlueprintType)
enum class EItemTier : uint8
{
	Common		UMETA(DisplayName = "Común"),
	Uncommon	UMETA(DisplayName = "Poco Común"),
	Rare		UMETA(DisplayName = "Raro"),
	Epic		UMETA(DisplayName = "Épico"),
	Legendary	UMETA(DisplayName = "Legendario")
};

/** Tipos de objetos que pueden existir en el inventario */
UENUM(BlueprintType)
enum class EItemType : uint8
{
	Weapon		UMETA(DisplayName = "Arma"),
	Ammo		UMETA(DisplayName = "Munición"),
	Armor		UMETA(DisplayName = "Armadura"),
	Medical		UMETA(DisplayName = "Médico"),
	Key			UMETA(DisplayName = "Llave"),
	Throwable	UMETA(DisplayName = "Arrojable"),
	Misc		UMETA(DisplayName = "Misceláneo")
};

/** Ranuras de equipamiento en el cuerpo del personaje */
UENUM(BlueprintType)
enum class EEquipmentSlot : uint8
{
	PrimaryWeapon	UMETA(DisplayName = "Arma Principal"),
	SecondaryWeapon	UMETA(DisplayName = "Arma Secundaria"),
	Holster			UMETA(DisplayName = "Pistolera"),
	Helmet			UMETA(DisplayName = "Casco"),
	BodyArmor		UMETA(DisplayName = "Armadura Corporal"),
	Vest			UMETA(DisplayName = "Chaleco"),
	Backpack		UMETA(DisplayName = "Mochila"),
	LeftPocket		UMETA(DisplayName = "Bolsillo Izquierdo"),
	RightPocket		UMETA(DisplayName = "Bolsillo Derecho")
};

// ============================================================
// ESTRUCTURAS COMPARTIDAS
// ============================================================

/** Datos de un objeto individual en el inventario */
USTRUCT(BlueprintType)
struct ZONAROJA_API FItemData
{
	GENERATED_BODY()

	/** Identificador único de esta instancia del objeto */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item")
	FGuid ID;

	/** ID de la definición del objeto (referencia a DataTable) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item")
	FName ItemDefinitionID;

	/** Cantidad en el stack (para objetos apilables como munición) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item")
	int32 StackCount = 1;

	/** Lista de IDs de accesorios adjuntos (para armas) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item")
	TArray<FGuid> AttachmentIDs;

	/** Constructor por defecto */
	FItemData()
	{
		ID = FGuid::NewGuid();
		ItemDefinitionID = NAME_None;
		StackCount = 1;
	}

	/** Constructor con ID de definición */
	FItemData(const FName& InDefinitionID, int32 InStackCount = 1)
	{
		ID = FGuid::NewGuid();
		ItemDefinitionID = InDefinitionID;
		StackCount = InStackCount;
	}

	/** Verifica si el item es válido */
	bool IsValid() const
	{
		return ID.IsValid() && ItemDefinitionID != NAME_None;
	}
};

/** Ranura individual en el inventario del jugador */
USTRUCT(BlueprintType)
struct ZONAROJA_API FInventorySlot
{
	GENERATED_BODY()

	/** Datos del objeto en esta ranura (inválido si está vacía) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Inventario")
	FItemData ItemData;

	/** Si la ranura está ocupada */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Inventario")
	bool bIsOccupied = false;

	/** Índice de la ranura en la cuadrícula del inventario */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Inventario")
	int32 SlotIndex = -1;

	/** Ancho que ocupa el item en la cuadrícula (en celdas) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Inventario")
	int32 ItemWidth = 1;

	/** Alto que ocupa el item en la cuadrícula (en celdas) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Inventario")
	int32 ItemHeight = 1;

	FInventorySlot() {}
};

/** Estructura de ranuras de equipamiento del personaje */
USTRUCT(BlueprintType)
struct ZONAROJA_API FEquipmentSlots
{
	GENERATED_BODY()

	/** Arma principal equipada (rifle, subfusil, etc.) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Equipamiento")
	FItemData PrimaryWeapon;

	/** Arma secundaria (rifle de respaldo) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Equipamiento")
	FItemData SecondaryWeapon;

	/** Pistola en pistolera */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Equipamiento")
	FItemData Holster;

	/** Casco protector */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Equipamiento")
	FItemData Helmet;

	/** Armadura corporal (chaleco antibalas) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Equipamiento")
	FItemData BodyArmor;

	/** Chaleco táctico (añade ranuras de inventario) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Equipamiento")
	FItemData Vest;

	/** Mochila (amplía el inventario principal) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Equipamiento")
	FItemData Backpack;

	/** Bolsillo izquierdo del pantalón */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Equipamiento")
	FItemData LeftPocket;

	/** Bolsillo derecho del pantalón */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Equipamiento")
	FItemData RightPocket;

	FEquipmentSlots() {}
};

/** Salud de una parte del cuerpo con armadura propia */
USTRUCT(BlueprintType)
struct ZONAROJA_API FBodyPartHealth
{
	GENERATED_BODY()

	/** Parte del cuerpo que representa esta estructura */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Salud")
	EBodyPart BodyPart = EBodyPart::Chest;

	/** Vida actual de esta parte del cuerpo */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Salud")
	float CurrentHealth = 100.0f;

	/** Vida máxima de esta parte del cuerpo */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Salud")
	float MaxHealth = 100.0f;

	/** Armadura actual que absorbe daño antes de penetrar */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Salud")
	float ArmorValue = 0.0f;

	/** Si esta parte está fracturada (reduce velocidad/puntería) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Salud")
	bool bIsFractured = false;

	/** Si esta parte está sangrando (daño por tiempo) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Salud")
	bool bIsBleeding = false;

	/** Tasa de sangrado en puntos de daño por segundo */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Salud")
	float BleedRate = 0.0f;

	FBodyPartHealth() {}

	FBodyPartHealth(EBodyPart InPart, float InMaxHealth)
		: BodyPart(InPart), CurrentHealth(InMaxHealth), MaxHealth(InMaxHealth)
	{}

	/** Devuelve el porcentaje de salud (0.0 - 1.0) */
	float GetHealthPercent() const
	{
		return (MaxHealth > 0.0f) ? (CurrentHealth / MaxHealth) : 0.0f;
	}

	/** Verifica si la parte está destruida */
	bool IsDestroyed() const
	{
		return CurrentHealth <= 0.0f;
	}
};

/** Datos de confirmación de impacto para validación en servidor */
USTRUCT(BlueprintType)
struct ZONAROJA_API FHitConfirmData
{
	GENERATED_BODY()

	/** ID único del disparo para evitar duplicados */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Disparo")
	FGuid ShotID;

	/** Posición del cañón en el momento del disparo */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Disparo")
	FVector MuzzleLocation = FVector::ZeroVector;

	/** Dirección del disparo desde el cañón */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Disparo")
	FRotator ShotRotation = FRotator::ZeroRotator;

	/** Actor objetivo del impacto (puede ser nulo si falló) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Disparo")
	TWeakObjectPtr<AActor> HitActor;

	/** Parte del cuerpo impactada */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Disparo")
	EBodyPart HitBodyPart = EBodyPart::Chest;

	/** Posición del impacto en el mundo */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Disparo")
	FVector HitLocation = FVector::ZeroVector;

	/** Timestamp del cliente en el momento del disparo */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Disparo")
	float ClientTimestamp = 0.0f;

	/** Daño calculado por el cliente (el servidor valida) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Disparo")
	float CalculatedDamage = 0.0f;

	FHitConfirmData()
	{
		ShotID = FGuid::NewGuid();
	}
};

/** Información de una zona de extracción activa en el mapa */
USTRUCT(BlueprintType)
struct ZONAROJA_API FExtractionZoneInfo
{
	GENERATED_BODY()

	/** Identificador único de la zona */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Extracción")
	FName ZoneID;

	/** Nombre visible para el jugador */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Extracción")
	FText DisplayName;

	/** Posición central de la zona en el mundo */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Extracción")
	FVector WorldLocation = FVector::ZeroVector;

	/** Si la zona está actualmente activa y disponible */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Extracción")
	bool bIsActive = false;

	/** Si la zona requiere un objeto especial (llave de extracción) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Extracción")
	bool bRequiresItem = false;

	/** ID del item requerido si bRequiresItem es true */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Extracción")
	FName RequiredItemID;

	/** Tiempo en segundos que tarda la extracción */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Extracción")
	float ExtractionTime = 5.0f;

	/** Número de jugadores actualmente en la zona */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Extracción")
	int32 PlayersInZone = 0;

	FExtractionZoneInfo()
		: ZoneID(NAME_None)
		, bIsActive(false)
		, bRequiresItem(false)
		, RequiredItemID(NAME_None)
		, ExtractionTime(5.0f)
		, PlayersInZone(0)
	{}
};
