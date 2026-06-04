# ARQUITECTURA TÉCNICA EN UNREAL ENGINE 5
## Proyecto: ZONA ROJA — Shooter de Extracción

**Versión:** 1.0  
**Motor:** Unreal Engine 5.3+  
**Modo de red:** Dedicado (Dedicated Server)  
**Lenguaje:** C++ con soporte Blueprint  

---

## 1. CONFIGURACIÓN DEL PROYECTO

### 1.1 Ajustes Recomendados en Project Settings

#### General
```
Project Name: ZonaRoja
Company Name: [Studio Name]
Default GameMode: BP_RaidGameMode (o C++ ARaidGameMode)
Default Map: Maps/MainMenu
Transition Map: Maps/Loading
```

#### Engine — Rendering
```
Default RHI: DirectX 12 (PC) / Vulkan (consola)
Anti-Aliasing: Temporal Super Resolution (TSR)
  - TSR Screen Percentage: 70 (upscale a 100)
Global Illumination: Lumen (Hardware Ray Tracing OFF en consola, ON en PC alta gama)
Reflections: Lumen Reflections
Shadows: Virtual Shadow Maps (VSM)
Nanite: Habilitado para mallas estáticas de entorno
Post Process: Tone Mapper: ACES, Exposure: Manual (Control total de iluminación)
Motion Blur: Desactivado (interfiere con lectura táctica)
Bloom: Habilitado, intensidad 0.6
Lens Flare: Desactivado (distractivo en combate)
```

#### Engine — Input
```
Input System: Enhanced Input System (obligatorio)
  DefaultInputComponentClass: EnhancedInputComponent
  DefaultPlayerInputClass: EnhancedPlayerInput
```

#### Engine — Collision
```
Canales de colisión personalizados:
  - ECC_Bullet (proyectiles/hitscan) — sin respuesta a Pawn
  - ECC_Interaction (línea de vista para interactuar objetos)
  - ECC_Visibility_Custom (oclusión para IA)
  - ECC_Penetration (verificación penetración de materiales)
```

#### Engine — Physics
```
Physics: Chaos Physics (UE5 default)
SubStepping: Habilitado
Max Physics Delta Time: 0.016667s
Substepping Max Steps: 4
Async Scene: Habilitado para mayor estabilidad
```

#### Plugins requeridos
```
- Enhanced Input (obligatorio)
- CommonUI (menus y HUD)
- Gameplay Abilities (GAS — Gameplay Ability System)
- Online Subsystem Steam / Online Subsystem EOS
- MetaSounds (audio procedural)
- Chaos Vehicles (si hay vehículos en el juego)
- Modular Gameplay (para arquitectura de componentes)
- Motion Warping (animaciones de vault/climb)
- Niagara (efectos de partículas: impactos, humo, sangre)
- Water (si hay cuerpos de agua)
- Landscape (terreno outdoor)
```

### 1.2 Configuración de Calidad

Crear perfiles de calidad en `Config/DefaultGameUserSettings.ini`:

```ini
[GameUserSettings]
ScalabilityQuality.ResolutionQuality=75
ScalabilityQuality.ViewDistanceQuality=3
ScalabilityQuality.AntiAliasingQuality=3
ScalabilityQuality.ShadowQuality=3
ScalabilityQuality.GlobalIlluminationQuality=2
ScalabilityQuality.ReflectionQuality=2
ScalabilityQuality.PostProcessQuality=3
ScalabilityQuality.TextureQuality=3
ScalabilityQuality.EffectsQuality=3
ScalabilityQuality.FoliageQuality=2
ScalabilityQuality.ShadingQuality=3
```

---

## 2. ESTRUCTURA DE CARPETAS (CONTENT BROWSER)

```
Content/
├── Characters/
│   ├── Player/
│   │   ├── Meshes/          ← Skeletal meshes del personaje jugador
│   │   ├── Animations/      ← AnimBPs, montajes, sequences
│   │   ├── Materials/       ← Instancias de material del cuerpo, cara, equipo
│   │   └── Blueprints/      ← BP_PlayerCharacter y derivados
│   ├── Enemies/
│   │   ├── PMC/
│   │   │   ├── Meshes/
│   │   │   ├── Animations/
│   │   │   └── Blueprints/  ← BP_PMC_Base, BP_PMC_Veteran, BP_PMC_Elite
│   │   ├── Mutants/
│   │   └── Bosses/
│   └── Shared/
│       ├── Animations/      ← Animaciones compartidas (idle, muerte genérica)
│       └── Materials/
│
├── Weapons/
│   ├── Rifles/
│   │   ├── AK103/
│   │   │   ├── Meshes/      ← SK_AK103, SK_AK103_FP (primera persona)
│   │   │   ├── Animations/  ← AM_AK103_Fire, AM_AK103_Reload, etc.
│   │   │   ├── Materials/
│   │   │   ├── Data/        ← DA_AK103 (DataAsset con estadísticas)
│   │   │   └── Blueprints/  ← BP_Weapon_AK103
│   │   └── M4A1/ ...
│   ├── SMGs/ ...
│   ├── Snipers/ ...
│   ├── Shotguns/ ...
│   ├── Pistols/ ...
│   ├── Attachments/
│   │   ├── Optics/          ← BP_Attachment_RedDot, BP_Attachment_ACOG, etc.
│   │   ├── Muzzle/          ← BP_Attachment_Suppressor, BP_Attachment_MuzzleBrake
│   │   ├── Grips/
│   │   └── Stocks/
│   └── Shared/
│       ├── Materials/       ← Master_Weapon material, instancias base
│       └── Blueprints/      ← BP_WeaponBase (clase padre de todas las armas)
│
├── Maps/
│   ├── Raid_Industrial/
│   │   ├── _Overview        ← Mapa principal del nivel
│   │   ├── Lighting/        ← Archivos de lightmass, sky atmospheres
│   │   ├── Volumes/         ← Volúmenes de audio, post-process, nav
│   │   └── Sublevels/       ← Niveles divididos por zona (streaming)
│   ├── Raid_Urban/
│   ├── MainMenu/
│   ├── Loading/
│   └── Shared/
│       ├── SpawnPoints/     ← Blueprints de spawn areas reutilizables
│       └── ExtractionZones/ ← BP_ExtractionZone
│
├── Environment/
│   ├── Modular/             ← Piezas modulares de construcción de niveles
│   │   ├── Industrial/
│   │   │   ├── Walls/
│   │   │   ├── Floors/
│   │   │   ├── Roofs/
│   │   │   ├── Pillars/
│   │   │   └── Props/
│   │   ├── Urban/
│   │   ├── Military/
│   │   └── Shared/
│   ├── Foliage/             ← Árboles, hierba, vegetación
│   ├── Decals/              ← Manchas de sangre, marcas de bala, suciedad
│   ├── Terrain/             ← Heightmaps, capas de terrain
│   └── LevelInstances/      ← Subconjuntos reutilizables (edificio genérico, etc.)
│
├── Gameplay/
│   ├── GameModes/           ← BP_RaidGameMode
│   ├── GameState/           ← BP_RaidGameState
│   ├── PlayerState/         ← BP_PlayerState_Raid
│   ├── PlayerController/    ← BP_PlayerController_Raid
│   ├── HUD/                 ← BP_HUD_Raid, WBP_HUD_Raid y widgets
│   ├── Inventory/           ← BP_InventoryComponent, WBP_Inventory, etc.
│   ├── AI/
│   │   ├── BehaviorTrees/   ← BT_PMC_Combat, BT_PMC_Patrol, etc.
│   │   ├── Blackboards/     ← BB_Enemy_Base
│   │   ├── Tasks/           ← BT_Task_TakeCover, BT_Task_ThrowGrenade
│   │   ├── Services/        ← BT_Service_UpdateEnemyFocus
│   │   ├── Decorators/      ← BT_Dec_HasAmmo, BT_Dec_IsInCombat
│   │   └── Controllers/     ← BP_AIController_PMC
│   ├── Abilities/           ← GAS: GameplayAbilities, Effects, Cues
│   │   ├── Player/
│   │   ├── Weapons/
│   │   └── Effects/
│   └── Items/
│       ├── Loot/            ← BP_LootContainer, BP_DroppedItem
│       └── Pickups/
│
├── UI/
│   ├── HUD/
│   ├── MainMenu/
│   ├── Inventory/
│   ├── Map/
│   ├── Trader/
│   └── Shared/              ← Widgets reutilizables (botones, iconos, etc.)
│
├── Audio/
│   ├── Weapons/
│   ├── Characters/
│   ├── Environment/
│   ├── Music/
│   └── MetaSounds/          ← Grafos de audio procedural
│
├── VFX/
│   ├── Weapons/             ← Niagara: fogonazos, casquillos, trazadores
│   ├── Impacts/             ← Impactos en distintos materiales
│   ├── Blood/
│   ├── Explosions/
│   └── Environment/         ← Humo, fuego, polvo
│
├── Data/
│   ├── DataTables/          ← DT_WeaponStats, DT_ItemList, DT_AmmoTypes
│   ├── DataAssets/          ← DA_WeaponBase, DA_ArmorTier, DA_ItemDefinition
│   └── Config/              ← Configuraciones de balance en DataAssets
│
└── _Dev/                    ← NUNCA en build de producción
    ├── [NombreDesarrollador]/  ← Sandbox por desarrollador
    └── TestMaps/
```

---

## 3. ARQUITECTURA DE CLASES PRINCIPALES

### 3.1 GameMode

```cpp
// ARaidGameMode : AGameModeBase
class ARaidGameMode : public AGameModeBase
{
    // Gestiona el ciclo de vida de la raid:
    // - Spawneo de jugadores
    // - Timer de raid (duración máxima)
    // - Spawneo de IA (oleadas, patrullas)
    // - Activación de zonas de extracción
    // - Resolución de fin de raid (todos muertos, tiempo agotado, extraídos)

    UPROPERTY(EditDefaultsOnly)
    float RaidDuration = 2700.f; // 45 minutos en segundos

    UPROPERTY(EditDefaultsOnly)
    int32 MaxPlayersPerRaid = 12;

    UPROPERTY(EditDefaultsOnly)
    int32 ActiveExtractionZones = 3;

    // Sólo existe en el servidor
    void BeginRaid();
    void OnPlayerExtracted(APlayerController* PC);
    void OnRaidTimerExpired();
    void SpawnInitialAI();
    void SelectActiveExtractionZones();
};
```

### 3.2 GameState

```cpp
// ARaidGameState : AGameStateBase
// Replicado a todos los clientes
class ARaidGameState : public AGameStateBase
{
    UPROPERTY(Replicated)
    float RaidTimeRemaining;

    UPROPERTY(Replicated)
    TArray<FExtractionZoneInfo> ActiveExtractionZones;

    UPROPERTY(Replicated)
    int32 TotalPlayersAlive;

    UPROPERTY(Replicated)
    int32 TotalPlayersExtracted;

    UPROPERTY(Replicated)
    ERaidPhase CurrentRaidPhase; // Insercion, Activa, Evacuacion, Fin

    // El servidor actualiza estas variables; los clientes las leen para el HUD
};
```

### 3.3 PlayerState

```cpp
// ARaidPlayerState : APlayerState
// Replicado, contiene info persistente del jugador durante la sesión
class ARaidPlayerState : public APlayerState
{
    UPROPERTY(Replicated)
    bool bIsAlive;

    UPROPERTY(Replicated)
    bool bHasExtracted;

    UPROPERTY(Replicated)
    int32 KillCount;

    UPROPERTY(ReplicatedUsing = OnRep_TeamID)
    int32 TeamID;

    // Datos de sesión (no replicados a enemigos por razones anti-cheat)
    // Gestionados server-side y en la base de datos
    TArray<FInventoryItem> RaidInventory;
    float TotalLootValue;
};
```

### 3.4 PlayerController

```cpp
// ARaidPlayerController : APlayerController
class ARaidPlayerController : public APlayerController
{
    // Gestión de input con Enhanced Input System
    UPROPERTY(EditDefaultsOnly)
    UInputMappingContext* IMC_OnFoot;

    UPROPERTY(EditDefaultsOnly)
    UInputMappingContext* IMC_InVehicle;

    UPROPERTY(EditDefaultsOnly)
    UInputMappingContext* IMC_UI;

    // Acciones Input
    UPROPERTY(EditDefaultsOnly)
    UInputAction* IA_Move;
    UPROPERTY(EditDefaultsOnly)
    UInputAction* IA_Look;
    UPROPERTY(EditDefaultsOnly)
    UInputAction* IA_Sprint;
    UPROPERTY(EditDefaultsOnly)
    UInputAction* IA_Crouch;
    UPROPERTY(EditDefaultsOnly)
    UInputAction* IA_Jump;
    UPROPERTY(EditDefaultsOnly)
    UInputAction* IA_Fire;
    UPROPERTY(EditDefaultsOnly)
    UInputAction* IA_ADS;
    UPROPERTY(EditDefaultsOnly)
    UInputAction* IA_Reload;
    UPROPERTY(EditDefaultsOnly)
    UInputAction* IA_Interact;
    UPROPERTY(EditDefaultsOnly)
    UInputAction* IA_OpenInventory;
    UPROPERTY(EditDefaultsOnly)
    UInputAction* IA_OpenMap;
    UPROPERTY(EditDefaultsOnly)
    UInputAction* IA_LeanLeft;
    UPROPERTY(EditDefaultsOnly)
    UInputAction* IA_LeanRight;
    UPROPERTY(EditDefaultsOnly)
    UInputAction* IA_ThrowGrenade;

    // RPC para acciones que necesitan verificación server-side
    UFUNCTION(Server, Reliable)
    void ServerRequestInteract(AActor* TargetActor);

    UFUNCTION(Server, Reliable)
    void ServerRequestPickupItem(FGuid ItemID);
};
```

---

## 4. ARQUITECTURA DE COMPONENTES DEL JUGADOR

El personaje jugador (BP_PlayerCharacter / APlayerCharacter) usa una arquitectura de componentes para separar responsabilidades:

### 4.1 HealthComponent

```cpp
// UHealthComponent : UActorComponent
// Gestiona vida, daño por zona corporal, estados de daño (sangrado, fractura)
class UHealthComponent : public UActorComponent
{
    UPROPERTY(ReplicatedUsing = OnRep_BodyParts)
    TMap<EBodyPart, FBodyPartHealth> BodyParts;
    // Cada BodyPart tiene: CurrentHP, MaxHP, bIsBleeding, bIsFractured

    UPROPERTY(Replicated)
    bool bIsConscious;

    UPROPERTY(Replicated)
    bool bIsDead;

    // Llamado por el servidor al recibir daño
    void ApplyDamage(float DamageAmount, EBodyPart HitZone, EDamageType DamageType);

    // Calcula HP total a partir de todas las zonas
    float GetTotalHealthPercent() const;

    // Aplica item médico a zona específica
    void ApplyMedicalItem(EBodyPart TargetZone, const FMedicalItemData& ItemData);

    // Delegates para notificar a otros sistemas
    FOnDeath OnDeath;
    FOnBodyPartDamaged OnBodyPartDamaged;
    FOnBleedingStarted OnBleedingStarted;
};
```

### 4.2 InventoryComponent

```cpp
// UInventoryComponent : UActorComponent
// Grid 2D de items, equipamiento, peso
class UInventoryComponent : public UActorComponent
{
    // Grid principal (mochila)
    UPROPERTY(Replicated)
    TArray<FInventorySlot> BackpackGrid; // Tamaño dinámico según mochila

    // Slots de equipamiento con posición fija
    UPROPERTY(Replicated)
    FEquipmentSlots Equipment;
    // Equipment contiene: PrimaryWeapon, SecondaryWeapon, Holster,
    //                     Helmet, BodyArmor, Vest, Backpack, LefPocket, RightPocket

    // Peso y límites
    UPROPERTY(Replicated)
    float CurrentWeight;

    float GetMaxWeight() const; // Calculado del nivel de fuerza + modificadores

    // Operaciones
    bool TryAddItem(const FItemData& Item, FIntPoint PreferredPosition = FIntPoint(-1,-1));
    bool RemoveItem(FGuid ItemID);
    bool MoveItem(FGuid ItemID, FIntPoint NewPosition);
    bool EquipItem(FGuid ItemID, EEquipmentSlot Slot);
    bool UnequipItem(EEquipmentSlot Slot);

    // Server RPCs para validación
    UFUNCTION(Server, Reliable)
    void ServerMoveItem(FGuid ItemID, FIntPoint NewPosition);

    UFUNCTION(Server, Reliable)
    void ServerDropItem(FGuid ItemID);
};
```

### 4.3 StaminaComponent

```cpp
// UStaminaComponent : UActorComponent
class UStaminaComponent : public UActorComponent
{
    UPROPERTY(ReplicatedUsing = OnRep_Stamina)
    float CurrentStamina;

    UPROPERTY(Replicated)
    float MaxStamina; // Modificada por habilidades y peso

    bool bIsExhausted; // Sin stamina → penalizaciones

    // Llamado desde MovementComponent
    void ConsumeStamina(float Amount);
    void RegenerateStamina(float DeltaTime);

    // Modificadores de equipo y habilidades
    void RecalculateMaxStamina();

    // Delegate para notificar a AnimBP y WeaponComponent
    FOnStaminaChanged OnStaminaChanged;
    FOnExhaustionStateChanged OnExhaustionStateChanged;
};
```

### 4.4 WeaponComponent

```cpp
// UWeaponComponent : UActorComponent
// Gestiona arma equipada, disparos, recarga, ADS
class UWeaponComponent : public UActorComponent
{
    UPROPERTY(Replicated)
    AWeaponBase* EquippedPrimaryWeapon;

    UPROPERTY(Replicated)
    AWeaponBase* EquippedSecondaryWeapon;

    UPROPERTY(Replicated)
    AWeaponBase* CurrentWeapon; // Arma activa en mano

    UPROPERTY(Replicated)
    bool bIsADS;

    UPROPERTY(Replicated)
    bool bIsReloading;

    void StartFire();
    void StopFire();
    void ToggleADS(bool bEnable);
    void StartReload();
    void SwapWeapon();

    // Hitscan raytrace con validación server-side
    UFUNCTION(Server, Reliable)
    void ServerValidateHit(FHitResult HitResult, FVector MuzzleLocation, FVector ShotDirection);
};
```

### 4.5 InteractionComponent

```cpp
// UInteractionComponent : UActorComponent
// Line trace de interacción con objetos del mundo
class UInteractionComponent : public UActorComponent
{
    float InteractionRange = 200.f; // 2 metros
    float InteractionTickRate = 0.1f; // 10 veces por segundo

    UPROPERTY(Replicated)
    AActor* CurrentInteractableActor; // Actor en el que apunta el jugador

    void TickInteractionTrace();
    void TryInteract();

    // Delegate para notificar al HUD
    FOnInteractableFound OnInteractableFound;
    FOnInteractableLost OnInteractableLost;
};
```

---

## 5. ESTRATEGIA DE REPLICACIÓN

### 5.1 Qué Replicar y Qué No

| Dato | ¿Replicado? | Razón |
|------|-------------|-------|
| HP por zona corporal | Sí (RepNotify) | HUD, animaciones de daño |
| Posición del jugador | Sí (movement replication) | Movimiento |
| Estado ADS | Sí (RepNotify) | AnimBP tercera persona |
| Inventario completo | No (server only + cliente dueño) | Anti-cheat, privacidad |
| Arma equipada (clase) | Sí | Otros jugadores ven el arma |
| Contenido del cargador | No (sólo dueño) | Información táctica privada |
| Posición de IA | Sí (owner: server) | Todos necesitan ver IA |
| Información de raid (timer, extracciones) | Sí (GameState) | HUD de todos |
| Chat de voz posición | Sí (solo si proximity) | Comunicación |

### 5.2 Patrones de Autoridad

```
Patrón 1: Server Authoritative (movimiento, daño, loot)
  Cliente → ServerRPC → Validación servidor → Replicar resultado

Patrón 2: Client Predictive (movimiento local)
  Cliente predice localmente → Envía a servidor → Servidor corrige si difiere

Patrón 3: Owner Only (inventario, UI privada)
  Servidor gestiona → Solo envía datos al dueño del PlayerState

Patrón 4: Multicast (efectos visuales/sonoros)
  Servidor dispara → NetMulticast RPC → Todos los clientes reproducen efecto
  (No crítico para gameplay, sin replay en caso de pérdida de paquete)
```

### 5.3 Relevancia de Red

```cpp
// En APlayerCharacter::PostInitializeComponents():
// Reducir replicación de actores lejanos
SetNetCullDistanceSquared(25000.f * 25000.f); // 250m

// Armas en suelo: replicación solo en rango de 100m
AWeaponPickup::SetNetCullDistanceSquared(10000.f * 10000.f);

// IA: replicada solo si está activa y dentro de 200m de algún jugador
// Usar AReplicationGraph o configurar IsNetRelevantFor() en AIController
```

---

## 6. NANITE Y LUMEN

### 6.1 Nanite — Cuándo Usarlo

**Usar Nanite en:**
- Edificios y estructuras modulares de entorno (muros, suelos, techos)
- Props estáticos grandes (vehículos abandonados, contenedores, maquinaria)
- Rocas, escombros, terreno detail meshes
- Cualquier malla con > 5000 triángulos que aparezca repetida en el nivel

**NO usar Nanite en:**
- Skeletal Meshes (personajes, armas animadas) — Nanite no soporta skeletal
- Vegetación altamente dinámica con WindAnim (usar Nanite + WPO con cuidado)
- Meshes con animaciones de vértice complejas
- Meshes con material translúcido
- Meshes pequeñísimas (< 500 tris) — overhead mayor que beneficio

**Configuración Nanite por malla:**
```
Preservar Area de Triángulo: 1.0 (calidad máxima para heroes)
Reducir a: 50% para props de fondo
Fallback Mesh LOD: LOD1 para plataformas sin Nanite (consola base)
```

### 6.2 Lumen — Configuración para Extracción Shooter

El juego combina interiores oscuros y exteriores abiertos, lo que requiere configuración cuidadosa de Lumen:

```cpp
// PostProcessVolume en interiores:
r.Lumen.DiffuseIndirect.Allow = 1
r.Lumen.Reflections.Allow = 1
r.LumenScene.SurfaceCacheMeshTargetsInWorldSpace = 1
// Limitar alcance en interiores para rendimiento:
r.Lumen.DiffuseIndirect.MaxTraceDistance = 5000 // 50m máx en interior
r.Lumen.Reflections.MaxRoughnessToTrace = 0.4

// PostProcessVolume en exteriores:
r.Lumen.DiffuseIndirect.MaxTraceDistance = 30000 // 300m exterior
r.Lumen.Reflections.MaxRoughnessToTrace = 0.6
```

**Regla práctica:** En interiores pequeños, Lumen puede tener artefactos. Añadir Reflection Captures (Sphere/Box) como fallback donde Lumen no alcance bien.

### 6.3 Virtual Shadow Maps (VSM)

```cpp
r.Shadow.Virtual.Enable = 1
r.Shadow.Virtual.Cache.Enable = 1 // Cache sombras estáticas = gran ahorro
r.Shadow.Virtual.MaxPhysicalPages = 2048 // Bajar a 1024 en consola
// IMPORTANTE: Marcar mallas estáticas como "Static Mobility"
// Las mallas Dynamic añaden coste significativo al VSM cache invalidation
```

---

## 7. SISTEMA DE INPUT (ENHANCED INPUT)

### 7.1 Input Mapping Contexts

```
IMC_OnFoot (prioridad 1): Movimiento, disparo, interacción, inventario
IMC_InVehicle (prioridad 1): Sobrescribe IMC_OnFoot en vehículo
IMC_UI (prioridad 2): Navegación de menús (bloquea gameplay input)
IMC_Spectator (prioridad 0): Modo espectador post-muerte
```

### 7.2 Input Actions y sus Modificadores

```
IA_Move:
  Type: Axis2D (Vector2D)
  Modificadores: Dead Zone (threshold 0.2), Swizzle Input Axis Values

IA_Look:
  Type: Axis2D
  Modificadores: Dead Zone, Scalar (para sensibilidad)
  Nota: Aplicar sensibilidad ADS vs Hip por código, no en IA

IA_Sprint:
  Type: Bool (Hold)
  Disparador: Hold

IA_Crouch:
  Type: Bool
  Disparador: Tap (toggle)

IA_Fire:
  Type: Bool (para auto: Hold; para semi: Tap)
  El arma decide si responde a Hold o solo Tap

IA_ADS:
  Type: Bool (Hold — soltar para dejar de apuntar)
  Nota: Opción de toggle en GameUserSettings

IA_Reload:
  Type: Bool (Tap)

IA_Interact:
  Type: Bool (Hold — 1.5s para interacciones lentas, Tap para rápidas)

IA_LeanLeft / IA_LeanRight:
  Type: Bool (Hold)
```

---

## 8. DATA ASSETS Y DATA TABLES

### 8.1 DataTable: DT_ItemDefinitions

Contiene todos los ítems del juego como rows:

```
Row Key: ITEM_AK103  (string, el ID único)
  DisplayName:        "AK-103"
  Description:        "Rifle de asalto ruso..."
  ItemType:           EItemType::Weapon
  Weight:             3800 (en gramos, para precisión sin float)
  GridSize:           FIntPoint(6, 2)  // 6 wide, 2 tall en inventario
  MaxStackSize:       1
  Tier:               EItemTier::Rare
  BaseValue:          12000 (en CZ)
  ThumbnailTexture:   T_UI_Item_AK103
  WorldMesh:          SM_AK103_World  (mesh cuando está en el suelo)
  WeaponDataAsset:    DA_Weapon_AK103 (null si no es arma)
  CanBeInsured:       true
  bIsQuestItem:       false
```

### 8.2 DataAsset: DA_WeaponData (por arma)

```cpp
UCLASS(BlueprintType)
class UWeaponDataAsset : public UPrimaryDataAsset
{
    UPROPERTY(EditAnywhere)
    FName ItemID; // "ITEM_AK103" — referencia a DT_ItemDefinitions

    UPROPERTY(EditAnywhere)
    EWeaponCategory Category; // Rifle, SMG, etc.

    UPROPERTY(EditAnywhere)
    EAmmoType AmmoType; // E762x39

    UPROPERTY(EditAnywhere)
    int32 BaseDamage;

    UPROPERTY(EditAnywhere)
    float FireRate; // RPM

    UPROPERTY(EditAnywhere)
    float MuzzleVelocity; // m/s

    UPROPERTY(EditAnywhere)
    float EffectiveRange; // cm en Unreal

    UPROPERTY(EditAnywhere)
    FRecoilData RecoilPattern; // Curva de retroceso

    UPROPERTY(EditAnywhere)
    TArray<EAttachmentSlot> AvailableAttachmentSlots;

    UPROPERTY(EditAnywhere)
    TSubclassOf<AWeaponBase> WeaponClass; // Blueprint del arma

    UPROPERTY(EditAnywhere)
    UAnimMontage* TPP_FireMontage; // Tercera persona

    UPROPERTY(EditAnywhere)
    UAnimMontage* FPP_FireMontage; // Primera persona

    UPROPERTY(EditAnywhere)
    USoundBase* FireSound_Far;
    UPROPERTY(EditAnywhere)
    USoundBase* FireSound_Near;
    UPROPERTY(EditAnywhere)
    USoundBase* ReloadSound_In;
    UPROPERTY(EditAnywhere)
    USoundBase* ReloadSound_Out;
};
```

### 8.3 DataTable: DT_AmmoTypes

```
Row Key: AMMO_762x39_FMJ
  DisplayName:       "7.62x39 FMJ"
  Caliber:           E762x39
  DamageMultiplier:  1.0
  PenetrationValue:  38
  FragmentationChance: 0.05
  InitialVelocity:   715 (m/s)
  DragCoefficient:   0.295
  MaxRange:          70000 (cm = 700m)
  DamageFalloffCurve: (referencia a UCurveFloat)
```

### 8.4 DataTable: DT_ArmorTiers

```
Row Key: ARMOR_LEVEL3
  DisplayName:       "Armadura Nivel 3"
  ArmorClass:        3
  MaxDurability:     60
  RepairCost:        3000 (CZ)
  BulletResistance:  50%  (vs FMJ estándar)
  PenetrationThreshold: 45 (balas con >45 penetración atraviesan)
  WeightGrams:       7200
  GridSize:          FIntPoint(5, 4)
```

---

## 9. ANIMATION BLUEPRINT

### 9.1 Estructura de AnimBP del Jugador (ABP_Player)

```
[AnimGraph Principal]
  ├── State Machine: Locomotion
  │     ├── State: Idle
  │     ├── State: Walk
  │     ├── State: Jog
  │     ├── State: Sprint
  │     ├── State: Crouch_Idle
  │     ├── State: Crouch_Walk
  │     ├── State: Prone_Idle
  │     ├── State: Prone_Crawl
  │     ├── State: InAir
  │     └── State: Landing
  │
  ├── Layered Blend per Bone: Upper Body
  │     ├── Slot: UpperBody_Additive (para disparos, impactos)
  │     └── Source: AimOffset (lean, mira arriba/abajo)
  │
  ├── Transform Bone: Lean (offset de inclinación)
  │
  └── Output Pose

[Variables del AnimBP]
  Speed (float)
  Direction (float -180 a 180)
  bIsADS (bool)
  bIsSprinting (bool)
  bIsCrouching (bool)
  bIsProne (bool)
  bIsInAir (bool)
  bIsReloading (bool)
  AimPitch (float)
  AimYaw (float)
  LeanAmount (float -1 a 1)
  CurrentWeaponType (EWeaponType)
  bIsAlive (bool)
```

### 9.2 Animation Montages Requeridos

Para cada arma principal, crear los siguientes montajes en slot `UpperBody`:

```
AM_[ArmaID]_Fire_Hip        — Disparo desde cadera
AM_[ArmaID]_Fire_ADS        — Disparo en ADS
AM_[ArmaID]_Reload_Normal   — Recarga con cargador vacío
AM_[ArmaID]_Reload_Tactical — Recarga táctica (queda una bala en cámara)
AM_[ArmaID]_Equip            — Sacar arma
AM_[ArmaID]_Unequip          — Guardar arma
AM_[ArmaID]_Inspect          — Inspeccionar arma (inactivo en combate)
AM_[ArmaID]_Jam_Clear        — Limpiar atasco (raro, mecánica avanzada)
```

---

*Ver WEAPONS_SYSTEM.md para detalles del sistema de armas y MULTIPLAYER_NETWORKING.md para configuración de red.*
