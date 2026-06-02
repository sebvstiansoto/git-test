# SISTEMA DE ARMAS — DISEÑO TÉCNICO DETALLADO
## Proyecto: ZONA ROJA

**Versión:** 1.0  
**Motor:** Unreal Engine 5 con C++ y Blueprints  

---

## 1. ESTRUCTURA DE DATOS DEL ARMA

### 1.1 Struct: FWeaponStats

Todas las estadísticas de un arma se almacenan en este struct, leído desde DataAssets:

```cpp
USTRUCT(BlueprintType)
struct FWeaponStats
{
    GENERATED_BODY()

    // ─── DAÑO ───────────────────────────────────────
    UPROPERTY(EditAnywhere, BlueprintReadOnly)
    int32 DamageBase;           // Daño al torso con munición estándar
    // El daño por zona se calcula: DamageBase * ZoneMultiplier (de DT_BodyZoneMultipliers)

    UPROPERTY(EditAnywhere, BlueprintReadOnly)
    float ArmorPenetration;     // 0-100: qué porcentaje de armadura ignora el proyectil
    // Fórmula: DamageFinal = DamageBase * (1 - ArmorAbsorption * (1 - ArmorPenetration/100))

    UPROPERTY(EditAnywhere, BlueprintReadOnly)
    UCurveFloat* DamageFalloffCurve; // Curva: X = distancia (cm), Y = multiplicador (0-1)

    UPROPERTY(EditAnywhere, BlueprintReadOnly)
    float FragmentationChance;  // 0-1: probabilidad de fragmentar en tejido blando

    // ─── CADENCIA Y DISPARO ─────────────────────────
    UPROPERTY(EditAnywhere, BlueprintReadOnly)
    float FireRate;             // Disparos por minuto (RPM)
    // TimeBetweenShots = 60.0 / FireRate

    UPROPERTY(EditAnywhere, BlueprintReadOnly)
    EFireMode DefaultFireMode;  // Semi, Burst3, Auto
    
    UPROPERTY(EditAnywhere, BlueprintReadOnly)
    TArray<EFireMode> AvailableFireModes; // Modos disponibles para el selector

    UPROPERTY(EditAnywhere, BlueprintReadOnly)
    int32 BurstCount;           // Número de disparos en modo burst (normalmente 3)

    UPROPERTY(EditAnywhere, BlueprintReadOnly)
    float BurstFireRate;        // RPM dentro del burst (generalmente más alto que full auto)

    // ─── BALÍSTICA ──────────────────────────────────
    UPROPERTY(EditAnywhere, BlueprintReadOnly)
    float MuzzleVelocity;       // m/s (velocidad inicial del proyectil)

    UPROPERTY(EditAnywhere, BlueprintReadOnly)
    float ProjectileMass;       // gramos (afecta drop gravitacional)

    UPROPERTY(EditAnywhere, BlueprintReadOnly)
    float BallisticCoefficient; // Coeficiente de resistencia aerodinámica (G1 model)

    UPROPERTY(EditAnywhere, BlueprintReadOnly)
    float MaxRange;             // cm (distancia donde el proyectil deja de ser letal, 1 daño)

    // ─── RETROCESO ──────────────────────────────────
    UPROPERTY(EditAnywhere, BlueprintReadOnly)
    UCurveVector* RecoilPatternCurve; 
    // X = eje horizontal (izquierda/derecha), Y = eje vertical, Z = cámara roll
    // Time = número de disparo consecutivo (0, 1, 2, ... hasta que se resetea)

    UPROPERTY(EditAnywhere, BlueprintReadOnly)
    float RecoilRecoverySpeed;  // Velocidad a la que el retroceso vuelve a origen (grados/s)

    UPROPERTY(EditAnywhere, BlueprintReadOnly)
    float RecoilRecoveryDelay;  // Segundos antes de que empiece la recuperación

    // ─── DISPERSIÓN (SPREAD) ────────────────────────
    UPROPERTY(EditAnywhere, BlueprintReadOnly)
    float SpreadBase;           // MOA base (desde cadera, quieto)

    UPROPERTY(EditAnywhere, BlueprintReadOnly)
    float SpreadADS;            // MOA en ADS (apuntado)

    UPROPERTY(EditAnywhere, BlueprintReadOnly)
    float SpreadMoving;         // MOA adicional al caminar
    
    UPROPERTY(EditAnywhere, BlueprintReadOnly)
    float SpreadSprinting;      // MOA adicional al correr (muy alto)

    UPROPERTY(EditAnywhere, BlueprintReadOnly)
    float SpreadPerShot;        // MOA que se añade por cada disparo consecutivo

    UPROPERTY(EditAnywhere, BlueprintReadOnly)
    float SpreadRecoveryRate;   // MOA/s de recuperación de spread

    // ─── CARGADOR Y MUNICIÓN ─────────────────────────
    UPROPERTY(EditAnywhere, BlueprintReadOnly)
    EAmmoType DefaultAmmoType;

    UPROPERTY(EditAnywhere, BlueprintReadOnly)
    int32 DefaultMagCapacity;   // Capacidad del cargador estándar

    UPROPERTY(EditAnywhere, BlueprintReadOnly)
    float ReloadTimeEmpty;      // Segundos para recargar con cámara vacía

    UPROPERTY(EditAnywhere, BlueprintReadOnly)
    float ReloadTimeTactical;   // Segundos para recarga táctica (queda bala en cámara)

    // ─── ERGONOMÍA Y MANEJO ─────────────────────────
    UPROPERTY(EditAnywhere, BlueprintReadOnly)
    float Ergonomics;           // 0-100: afecta velocidad de ADS y fatiga de arma

    UPROPERTY(EditAnywhere, BlueprintReadOnly)
    float ADSTime;              // Segundos para llegar a ADS completo

    UPROPERTY(EditAnywhere, BlueprintReadOnly)
    float WeaponSwayMultiplier; // Multiplicador de sway base (por stamina, movimiento)

    UPROPERTY(EditAnywhere, BlueprintReadOnly)
    float WeightGrams;          // Peso del arma sin cargador (afecta peso total del loadout)
};
```

### 1.2 Tabla de Estadísticas por Arma

| Arma | Daño base | RPM | Velocidad m/s | Penetración | MOA base | Peso (g) |
|------|-----------|-----|---------------|-------------|----------|----------|
| AK-103 | 58 | 600 | 715 | 38 | 1.8 | 3800 |
| M4A1 | 50 | 800 | 910 | 35 | 1.4 | 3400 |
| HK416 | 52 | 850 | 920 | 36 | 1.2 | 3600 |
| MP5 | 38 | 800 | 380 | 22 | 2.0 | 2500 |
| PP-19 Bizon | 36 | 650 | 360 | 20 | 2.2 | 2700 |
| Vector .45 | 44 | 1200 | 270 | 18 | 2.8 | 1600 |
| FAL | 72 | 650 | 840 | 52 | 1.0 | 4700 |
| SVD | 85 | Semi | 830 | 55 | 0.8 | 4300 |
| Mossberg 590 | 20×9 | Semi | 370 | 15 | n/a | 3000 |
| PKM | 55 | 700 | 855 | 42 | 2.5 | 8200 |
| Glock 17 | 40 | Semi | 375 | 20 | 3.0 | 625 |

---

## 2. SISTEMA DE ADJUNTOS (ATTACHMENTS)

### 2.1 Slots Disponibles

```cpp
UENUM(BlueprintType)
enum class EAttachmentSlot : uint8
{
    None            UMETA(DisplayName = "Ninguno"),
    Optic           UMETA(DisplayName = "Óptica"),
    Muzzle          UMETA(DisplayName = "Boca del cañón"),
    UnderBarrel     UMETA(DisplayName = "Bajo cañón"),
    SideRail        UMETA(DisplayName = "Riel lateral"),
    Stock           UMETA(DisplayName = "Culata"),
    Barrel          UMETA(DisplayName = "Cañón"),
    Magazine        UMETA(DisplayName = "Cargador"),
    Grip            UMETA(DisplayName = "Empuñadura"),
    Charging        UMETA(DisplayName = "Llave de armado"),
};
```

### 2.2 Struct: FAttachmentData

```cpp
USTRUCT(BlueprintType)
struct FAttachmentData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere)
    FName ItemID;               // ID en DT_ItemDefinitions

    UPROPERTY(EditAnywhere)
    EAttachmentSlot Slot;

    // Modificadores estadísticos (se suman/multiplican a FWeaponStats)
    UPROPERTY(EditAnywhere)
    float DamageModifier;       // ej: -5 (supresores reducen daño levemente)

    UPROPERTY(EditAnywhere)
    float RecoilModifier;       // ej: -0.2 (compensador: -20% retroceso vertical)

    UPROPERTY(EditAnywhere)
    float SpreadModifier;       // ej: -0.3 MOA (bipod reduce spread significativamente)

    UPROPERTY(EditAnywhere)
    float ErgonomicsModifier;   // ej: +10 (empuñadura mejora ergonomía)

    UPROPERTY(EditAnywhere)
    float WeightModifier;       // ej: +300 (gramos añadidos)

    UPROPERTY(EditAnywhere)
    float ADSTimeModifier;      // ej: -0.05s (óptica ligera mejora ADS)

    UPROPERTY(EditAnywhere)
    float SoundModifier;        // ej: -0.7 (supresor reduce 70% volumen)

    UPROPERTY(EditAnywhere)
    float MuzzleVelocityModifier; // ej: -50 m/s (cañón corto reduce velocidad)

    UPROPERTY(EditAnywhere)
    bool bGrantsSuppression;    // Supresor: activa sistema de supresión de sonido

    UPROPERTY(EditAnywhere)
    float NightVisionMagnification; // Para PVS-14, etc.

    // Compatibilidad: lista de armas con las que es compatible
    UPROPERTY(EditAnywhere)
    TArray<FName> CompatibleWeapons; // Lista de ItemIDs de armas compatibles

    // Visual
    UPROPERTY(EditAnywhere)
    UStaticMesh* AttachmentMesh;

    UPROPERTY(EditAnywhere)
    FName SocketName;           // Socket en el arma donde se attachea el mesh
};
```

### 2.3 Catálogo de Adjuntos

#### Ópticas
| Nombre | Tipo | Aumento | Mod Ergonomía | Mod ADS | Peso (g) |
|--------|------|---------|----------------|---------|----------|
| Punto rojo Kobra | Reflejo | 1x | +8 | -0.03s | 270 |
| Holosight 552 | Holográfico | 1x | +10 | -0.04s | 380 |
| ACOG TA31 | Telescópico | 4x | -5 | +0.06s | 420 |
| Vudu 1-6x24 | Variable 1-6x | 1-6x | +5 | +0.02s | 530 |
| March 10x | Francotirador | 10x | -15 | +0.15s | 680 |
| PVS-14 | Visión nocturna | 1x | -3 | +0.05s | 450 |

#### Bocas de cañón
| Nombre | Mod Retroceso V | Mod Retroceso H | Mod Sonido | Mod Vel | Peso (g) |
|--------|----------------|-----------------|------------|---------|----------|
| Flash hider A2 | -10% | 0% | -5% | 0 | 85 |
| Compensador 7.62 | -25% | +10% (más) | +5% | 0 | 110 |
| Freno de boca AK | -35% V | -20% H | +15% | 0 | 145 |
| Supresor 7.62 PBS-1 | -5% | -5% | -65% | -40 m/s | 590 |
| Supresor 9mm PBS-4 | -8% | -8% | -72% | -20 m/s | 320 |

---

## 3. BALÍSTICA DETALLADA

### 3.1 Modelo de Proyectil en Vuelo

Cada disparo genera un `AProjectileActor` o un trace asíncrono (según la distancia y necesidad):

```cpp
// Para distancias < 100m: Hitscan con compensación de gravedad lineal
// Para distancias > 100m: Proyectil físico (UProjectileMovementComponent)

// Cálculo de caída gravitacional (bullet drop):
// drop = 0.5 * gravity * (distance / velocity)²
// A 300m con AK-103 (715 m/s): drop ≈ 8.6 cm
// A 500m: drop ≈ 23.9 cm

float CalculateBulletDrop(float DistanceMeters, float MuzzleVelocityMS)
{
    const float gravity = 9.81f;
    float timeOfFlight = DistanceMeters / MuzzleVelocityMS;
    return 0.5f * gravity * timeOfFlight * timeOfFlight; // metros
}
```

### 3.2 Penetración de Materiales

```cpp
UENUM(BlueprintType)
enum class EMaterialPenetration : uint8
{
    None,           // Concreto grueso, piedra: no penetra (ninguna bala estándar)
    Low,            // Vidrio, drywall fino: calibres pistola y +
    Medium,         // Madera gruesa, metal delgado: calibres rifle
    High,           // Metal medio, vidrio balístico fino: calibres rifle de batalla
    VeryHigh,       // Paredes de ladrillo delgadas: .50 BMG, SLAP
};

struct FMaterialPenetrationData
{
    EMaterialPenetration PenetrationLevel; // Nivel mínimo para atravesar
    float VelocityLossPercent;  // % de velocidad que pierde el proyectil al atravesar
    float DamageLossPercent;    // % de daño perdido al penetrar
    float MaxThicknessCM;       // Grosor máximo que puede atravesar
};
```

**Tabla de penetración de materiales:**

| Material | Nivel requerido | Pérdida vel | Pérdida daño | Grosor máx (cm) |
|----------|----------------|-------------|--------------|-----------------|
| Vidrio (5mm) | Low | 5% | 15% | 1cm |
| Drywall (12mm) | Low | 8% | 20% | 3cm |
| Madera (25mm) | Medium | 20% | 35% | 10cm |
| Chapa metálica (2mm) | Medium | 25% | 40% | 0.5cm |
| Ladrillo (10cm) | High | 60% | 70% | 12cm |
| Concreto (20cm) | VeryHigh | 90% | 95% | 5cm |

### 3.3 Sistema de Supresión

Cuando un disparo pasa a menos de 200cm del jugador, se activa la supresión:

```cpp
struct FSuppresionEffect
{
    float SuppresionLevel;       // 0-1, basado en proximidad y número de disparos
    float AimShakeIntensity;     // Cantidad de sacudida de cámara
    float SpreadIncrease;        // MOA añadido por supresión
    float SuppresionDecayRate;   // Cuánto baja la supresión por segundo
    bool bAuditoryRinging;       // Pitido auditivo (sin supresor a menos de 5m)
};

// Activación:
// - Disparo pasa a < 50cm: SuppresionLevel += 0.4
// - Disparo pasa a 50-200cm: SuppresionLevel += 0.15
// - Decay: -0.1/s cuando no hay disparos cercanos
// - SuppresionLevel máximo: 1.0
```

### 3.4 Efectos del Supresor

Un supresor no hace el arma inaudible, sino que:
1. **Reduce el fogonazo:** Elimina el flash en la boca del cañón
2. **Reduce el volumen del disparo:** -65-75% según el supresor y calibre
3. **Cambia la firma sónica:** El sonido del disparo cambia de tono (más sordo, grave)
4. **No elimina el crack sónico del proyectil:** A velocidades supersónicas (> 343 m/s), el crack sónico sigue siendo audible. Munición subsónica elimina el crack.
5. **Dificulta la localización de la fuente de disparo:** A > 50m con supresor, la dirección se vuelve ambigua

---

## 4. ANIMATION BLUEPRINT PARA ARMAS

### 4.1 ABP_Weapon_Rifle (AnimBP de Primera Persona)

El arma en primera persona tiene su propio AnimBP que se sincroniza con el personaje:

```
[AnimGraph — ABP_Weapon_Rifle]
  │
  ├── Slot: Weapon_Additive
  │     → Montajes de disparo, recarga, jam
  │
  ├── AimOffset: Weapon_AimOffset
  │     → Reacciona a la dirección de la cámara (pitch/yaw)
  │
  ├── Layered Blend: Procedural Recoil
  │     → Animación procedural de retroceso (aplicada sobre todo lo demás)
  │
  └── Output Pose
```

**Variables del ABP_Weapon:**
```cpp
float WeaponSway_X;     // Sway horizontal (desde StaminaComponent, movimiento)
float WeaponSway_Y;     // Sway vertical
float RecoilOffset_X;   // Offset acumulado de retroceso horizontal
float RecoilOffset_Y;   // Offset acumulado de retroceso vertical
float ADSAlpha;         // 0 = cadera, 1 = ADS completo
bool bIsReloading;
bool bIsSprinting;
float SprintLean;       // Inclinación del arma al correr
EWeaponFireMode FireMode;
```

### 4.2 Montajes de Animación Requeridos

Nomenclatura y descripción de cada montaje:

```
AM_[Arma]_Fire_Hip
  Duración: 0.1s (un disparo)
  Slot: UpperBody_FPP
  Descripción: Sacudida hacia atrás y arriba. Retroceso visual.
  Notify: SpawnCasing (expulsar casquillo), SpawnMuzzleFlash

AM_[Arma]_Fire_ADS
  Duración: 0.08s (levemente más controlado en ADS)
  Slot: UpperBody_FPP
  Descripción: Retroceso más controlado, menos sway lateral

AM_[Arma]_Reload_Empty
  Duración: [ver tabla por arma]
  Slot: UpperBody_FPP
  Notifies:
    - 0.5s: EjectMagazine (desaparece cargador actual, aparece en mano izquierda)
    - 1.2s: InsertMagazine (cargador nuevo en arma, desaparece de mano)
    - 1.8s: ChargingHandle (armar el arma, cargar primera bala)
    - Al final: ReloadComplete (habilitar disparo)

AM_[Arma]_Reload_Tactical
  Duración: [ver tabla − recargas tácticas son más rápidas]
  Notifies:
    - EjectMagazine, InsertMagazine
    - Sin ChargingHandle (ya hay bala en recámara)

AM_[Arma]_Equip
  Duración: 0.6-1.0s
  Slot: UpperBody_FPP
  Descripción: Sacar el arma del holster/espalda

AM_[Arma]_Unequip
  Duración: 0.4-0.6s
  Descripción: Guardar el arma

AM_[Arma]_Sprint_Start
  Descripción: Transición a posición de sprint (arma inclinada)

AM_[Arma]_Sprint_End
  Descripción: Volver a posición de combate desde sprint
```

### 4.3 Retroceso Procedural

El retroceso se aplica como curvas procedurales, no como animaciones keyframed:

```cpp
// En la clase de componente del arma o en el PlayerController:
void UWeaponComponent::ApplyRecoil()
{
    if (!CurrentWeapon) return;
    
    const FWeaponStats& Stats = CurrentWeapon->GetWeaponStats();
    int32 ShotIndex = FMath::Clamp(ShotsFiredConsecutive, 0, 30);
    
    // Leer la curva de retroceso para este disparo
    FVector RecoilVector = Stats.RecoilPatternCurve->GetVectorValue(ShotIndex);
    
    // Aplicar modificadores de adjuntos
    RecoilVector.Y *= (1.0f - ComputedRecoilModifier); // Vertical
    RecoilVector.X *= (1.0f - ComputedHorizontalModifier); // Horizontal
    
    // Modificador de supresión (más retroceso si está exhausto)
    float StaminaFactor = 1.0f + (StaminaComponent->IsExhausted() ? 0.4f : 0.0f);
    RecoilVector *= StaminaFactor;
    
    // Aplicar al controlador del jugador
    PlayerController->AddPitchInput(-RecoilVector.Y * RecoilMultiplier);
    PlayerController->AddYawInput(RecoilVector.X * RecoilMultiplier);
    
    ShotsFiredConsecutive++;
}

// Timer de reset de retroceso:
void UWeaponComponent::StartRecoilRecovery()
{
    // Tras RecoilRecoveryDelay segundos sin disparar, smooth recovery a origen
    GetWorld()->GetTimerManager().SetTimer(
        RecoilRecoveryTimer,
        this, &UWeaponComponent::TickRecoilRecovery,
        0.016f, true, Stats.RecoilRecoveryDelay
    );
}
```

---

## 5. JERARQUÍA DE CLASES DE ARMA

### 5.1 Árbol de Herencia

```
AWeaponBase (C++ base)
├── AFirearmBase (C++ — armas de fuego)
│   ├── ARifle (C++ o Blueprint)
│   │   ├── BP_Weapon_AK103
│   │   ├── BP_Weapon_M4A1
│   │   └── BP_Weapon_HK416
│   ├── ASMG
│   │   ├── BP_Weapon_MP5
│   │   └── BP_Weapon_Vector
│   ├── ABattleRifle
│   │   └── BP_Weapon_FAL
│   ├── ASniperRifle
│   │   └── BP_Weapon_SVD
│   ├── AShotgun
│   │   └── BP_Weapon_Mossberg590
│   ├── ALMG
│   │   └── BP_Weapon_PKM
│   └── APistol
│       ├── BP_Weapon_Glock17
│       └── BP_Weapon_DesertEagle
│
├── AThrowableBase (C++ — granadas, armas arrojadizas)
│   ├── BP_Grenade_Frag
│   ├── BP_Grenade_Smoke
│   └── BP_Grenade_Flash
│
└── AMeleeBase (C++ — cuerpo a cuerpo)
    ├── BP_Melee_Knife
    └── BP_Melee_Tomahawk
```

### 5.2 AWeaponBase — Interfaz Pública

```cpp
class AWeaponBase : public AActor
{
public:
    // Ciclo de vida
    virtual void StartFire() {}
    virtual void StopFire() {}
    virtual void Reload() {}
    virtual void Equip(ACharacter* NewOwner) {}
    virtual void Unequip() {}

    // Información
    virtual FWeaponStats GetWeaponStats() const { return FWeaponStats(); }
    virtual FName GetItemID() const { return NAME_None; }
    virtual bool CanFire() const { return false; }
    virtual bool CanReload() const { return false; }
    virtual int32 GetCurrentAmmoInMag() const { return 0; }
    virtual int32 GetAmmoReserve() const { return 0; }

    // Adjuntos
    virtual bool AttachAccessory(const FAttachmentData& AttachmentData) { return false; }
    virtual void RemoveAccessory(EAttachmentSlot Slot) {}
    virtual const FAttachmentData* GetAttachment(EAttachmentSlot Slot) const { return nullptr; }

    // Replicación
    UPROPERTY(Replicated)
    ACharacter* OwnerCharacter;

protected:
    // SubComponents comunes
    UPROPERTY(VisibleAnywhere)
    USkeletalMeshComponent* WeaponMesh;

    UPROPERTY(VisibleAnywhere)
    USkeletalMeshComponent* WeaponMesh_FPP; // Mesh de primera persona

    // Datos del arma
    UPROPERTY(EditDefaultsOnly)
    UWeaponDataAsset* WeaponData;
};
```

### 5.3 AFirearmBase — Funcionalidad de Disparo

```cpp
class AFirearmBase : public AWeaponBase
{
public:
    void StartFire() override;
    void StopFire() override;
    void Reload() override;

protected:
    // Lógica de disparo
    void FireShot();
    virtual void HandleHitscan(FVector MuzzleLocation, FVector ShootDirection);
    virtual void SpawnProjectile(FVector MuzzleLocation, FVector ShootDirection);
    
    void EjectCasing();
    void SpawnMuzzleFlash();
    void PlayFireSound();
    
    // Estado de munición (replicado al dueño)
    UPROPERTY(ReplicatedUsing = OnRep_AmmoState)
    FAmmoState AmmoState;
    // FAmmoState: int32 AmmoInMag, int32 AmmoReserve, bool bIsChamberLoaded

    // Timer de cadencia de fuego
    FTimerHandle FireRateTimer;
    
    // Estado de retroceso
    int32 ShotsFiredConsecutive;
    FTimerHandle RecoilRecoveryTimer;
    
    // Adjuntos activos
    TMap<EAttachmentSlot, FAttachmentData> ActiveAttachments;
    
    // Estadísticas calculadas (base + modificadores de adjuntos)
    FWeaponStats ComputedStats;
    void RecalculateStats(); // Llamar cuando se cambia un adjunto

    // Server RPC para validación de disparo
    UFUNCTION(Server, Reliable)
    void ServerConfirmHit(const FHitConfirmData& HitData);

    // Multicast para efectos visuales/sonoros de disparo
    UFUNCTION(NetMulticast, Unreliable)
    void MulticastFireEffects(FVector MuzzleLocation, FRotator MuzzleRotation);
};
```

---

## 6. VALIDACIÓN SERVER-SIDE DE DISPAROS

### 6.1 Flujo de Hit Confirmation

```
CLIENTE                              SERVIDOR
  │                                      │
  │── Dispara localmente (visual) ──────►│
  │   Calcula hit result local           │
  │   Guarda snapshot del estado         │
  │                                      │
  │── ServerConfirmHit RPC ─────────────►│
  │   (HitResult, Timestamp,             │   Rewinda el mundo al timestamp
  │    ShootDir, MuzzlePos)              │   del cliente (lag compensation)
  │                                      │
  │                                      │   Verifica:
  │                                      │   1. ¿El target estaba en esa posición?
  │                                      │   2. ¿La dirección de disparo es válida?
  │                                      │   3. ¿Hay line of sight entre muzzle y hit?
  │                                      │   4. ¿El daño calculado es coherente?
  │                                      │
  │◄── Aplicar daño al target ──────────│ (Solo si validación pasa)
  │◄── Feedback de hit al shooter ──────│
```

### 6.2 Tolerancias de Validación

```cpp
struct FHitValidationConfig
{
    float MaxAngleTolerance = 5.0f;   // grados: tolerancia de ángulo de disparo
    float MaxDistanceTolerance = 50.0f; // cm: tolerancia en posición del impacto
    float MaxTimestampAge = 0.5f;      // segundos: rechazar hits con timestamp muy antiguo
    float MaxLagCompensationTime = 0.25f; // segundos máx de rewind para lag compensation
};
```

---

## 7. SISTEMA DE ATASCOS (JAM)

Mecánica avanzada que añade tensión en combate:

```cpp
struct FJamData
{
    float JamChancePerShot;  // Probabilidad de atasco por disparo (0-1)
    // Se calcula como: BaseJamChance + (DurabilityFactor) + (AmmoQualityFactor)
    // DurabilityFactor: arma deteriorada → mayor jam chance
    // AmmoQualityFactor: munición de baja calidad → mayor jam chance

    // Ejemplo AK-103 en buenas condiciones con munición estándar: 0.0005 (0.05%)
    // AK-103 deteriorada con munición reciclada: 0.02 (2%)
};

// Al producirse un atasco:
// 1. El arma emite sonido de click vacío
// 2. Icono de atasco en HUD (mantenida presión de recarga para limpiar)
// 3. Animación AM_[Arma]_Jam_Clear (1.5-3.0 segundos)
// 4. El jugador es vulnerable durante el clearance
```

---

*Ver MULTIPLAYER_NETWORKING.md para la implementación de replicación de armas y lag compensation.*
