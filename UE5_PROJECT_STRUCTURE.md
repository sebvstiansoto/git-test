# ESTRUCTURA DEL PROYECTO UE5
## Árbol de Carpetas del Content Browser — ZONA ROJA

**Versión:** 1.0  
**Motor:** Unreal Engine 5.3+  

---

## 1. ÁRBOL COMPLETO DE CARPETAS

A continuación se detalla la estructura completa de la carpeta `Content/` dentro del proyecto UE5. Cada carpeta tiene un propósito específico y convenciones de nombrado.

```
Content/
│
├── _Core/                          ← Sistemas de base del juego (no modificar sin autorización)
│   ├── GameModes/                  ← Blueprint de GameMode principal y variantes
│   │   ├── BP_RaidGameMode.uasset
│   │   └── BP_MainMenuGameMode.uasset
│   ├── GameStates/
│   │   └── BP_RaidGameState.uasset
│   ├── PlayerStates/
│   │   └── BP_RaidPlayerState.uasset
│   ├── PlayerControllers/
│   │   ├── BP_PlayerController_Raid.uasset
│   │   └── BP_PlayerController_Spectator.uasset
│   ├── GameInstances/
│   │   └── BP_ZonaRojaGameInstance.uasset
│   └── SaveGames/
│       ├── BP_SaveGame_PlayerProgress.uasset
│       └── BP_SaveGame_Settings.uasset
│
├── Characters/
│   │
│   ├── Player/                     ← Todo lo relacionado con el personaje jugador
│   │   ├── Meshes/
│   │   │   ├── SK_Player_Base.uasset          ← Skeletal mesh base (sin ropa/equipo)
│   │   │   ├── SK_Player_Base_PhysicsAsset.uasset
│   │   │   └── SK_Player_Base_Skeleton.uasset ← Skeleton asset compartido
│   │   ├── Blueprints/
│   │   │   └── BP_PlayerCharacter.uasset      ← Clase principal del personaje
│   │   ├── Animations/
│   │   │   ├── AnimBP/
│   │   │   │   ├── ABP_Player_TPP.uasset      ← AnimBP tercera persona
│   │   │   │   └── ABP_Player_FPP.uasset      ← AnimBP primera persona (brazos)
│   │   │   ├── AimOffsets/
│   │   │   │   ├── AO_Rifle_Hip.uasset
│   │   │   │   └── AO_Rifle_ADS.uasset
│   │   │   ├── BlendSpaces/
│   │   │   │   ├── BS_Locomotion_Walk.uasset
│   │   │   │   └── BS_Locomotion_Crouch.uasset
│   │   │   ├── Montages/
│   │   │   │   ├── AM_Player_Vault.uasset
│   │   │   │   ├── AM_Player_Death_Forward.uasset
│   │   │   │   ├── AM_Player_Death_Back.uasset
│   │   │   │   └── AM_Player_Interact.uasset
│   │   │   └── Sequences/                     ← Animaciones de cinemática
│   │   ├── Materials/
│   │   │   ├── MI_Player_Skin_Base.uasset     ← Material Instance de piel
│   │   │   ├── MI_Player_Gear_Default.uasset
│   │   │   └── MI_Player_Eyes.uasset
│   │   └── Cosmetics/                         ← Variantes de aspecto (skins)
│   │       ├── Head/
│   │       ├── Body/
│   │       └── Gear/
│   │
│   ├── Enemies/
│   │   ├── PMC/                               ← Mercenario IA
│   │   │   ├── Meshes/
│   │   │   │   ├── SK_PMC_Base.uasset
│   │   │   │   ├── SK_PMC_Veteran.uasset
│   │   │   │   └── SK_PMC_Elite.uasset
│   │   │   ├── Blueprints/
│   │   │   │   ├── BP_PMC_Novice.uasset
│   │   │   │   ├── BP_PMC_Veteran.uasset
│   │   │   │   └── BP_PMC_Elite.uasset
│   │   │   ├── Animations/
│   │   │   │   └── ABP_PMC_Base.uasset
│   │   │   └── Materials/
│   │   ├── Mutants/
│   │   │   ├── Meshes/
│   │   │   │   └── SK_Mutant_Standard.uasset
│   │   │   ├── Blueprints/
│   │   │   │   └── BP_Mutant.uasset
│   │   │   └── Animations/
│   │   │       └── ABP_Mutant.uasset
│   │   └── Bosses/
│   │       └── Barkov/
│   │           ├── SK_Barkov.uasset
│   │           ├── BP_Boss_Barkov.uasset
│   │           └── ABP_Barkov.uasset
│   │
│   └── Shared/
│       ├── Animations/                        ← Animaciones reutilizadas entre chars
│       │   └── AM_Shared_Death_Generic.uasset
│       ├── Materials/
│       │   └── M_Character_Master.uasset      ← Master material de personajes
│       └── Components/                        ← Componentes Blueprint compartidos
│           ├── BP_HealthComponent.uasset
│           ├── BP_InventoryComponent.uasset
│           ├── BP_StaminaComponent.uasset
│           └── BP_WeaponComponent.uasset
│
├── Weapons/
│   │
│   ├── _Base/                                 ← Clases y materiales base
│   │   ├── BP_WeaponBase.uasset               ← Blueprint base de todas las armas
│   │   ├── BP_FirearmBase.uasset
│   │   ├── M_Weapon_Master.uasset             ← Master material de armas
│   │   └── DT_WeaponSockets.uasset            ← DataTable de sockets por arma
│   │
│   ├── Rifles/
│   │   ├── AK103/
│   │   │   ├── SK_AK103.uasset                ← Skeletal mesh (tercera persona)
│   │   │   ├── SK_AK103_FPP.uasset            ← Skeletal mesh (primera persona)
│   │   │   ├── SK_AK103_Skeleton.uasset
│   │   │   ├── BP_Weapon_AK103.uasset
│   │   │   ├── DA_Weapon_AK103.uasset         ← DataAsset con stats
│   │   │   ├── Materials/
│   │   │   │   ├── MI_AK103_Default.uasset
│   │   │   │   └── MI_AK103_Desert.uasset
│   │   │   └── Animations/
│   │   │       ├── ABP_AK103_FPP.uasset
│   │   │       ├── AM_AK103_Fire_Hip.uasset
│   │   │       ├── AM_AK103_Fire_ADS.uasset
│   │   │       ├── AM_AK103_Reload_Empty.uasset
│   │   │       ├── AM_AK103_Reload_Tactical.uasset
│   │   │       ├── AM_AK103_Equip.uasset
│   │   │       └── AM_AK103_Inspect.uasset
│   │   ├── M4A1/ (misma estructura que AK103)
│   │   └── HK416/ (misma estructura que AK103)
│   │
│   ├── SMGs/
│   │   ├── MP5/ (misma estructura)
│   │   └── Vector/ (misma estructura)
│   │
│   ├── SniperRifles/
│   │   └── SVD/ (misma estructura)
│   │
│   ├── Shotguns/
│   │   └── Mossberg590/ (misma estructura)
│   │
│   ├── LMGs/
│   │   └── PKM/ (misma estructura)
│   │
│   ├── Pistols/
│   │   ├── Glock17/ (misma estructura)
│   │   └── DesertEagle/ (misma estructura)
│   │
│   ├── Attachments/
│   │   ├── _Base/
│   │   │   └── BP_AttachmentBase.uasset
│   │   ├── Optics/
│   │   │   ├── SM_Optic_RedDot_Kobra.uasset
│   │   │   ├── SM_Optic_Holosight.uasset
│   │   │   ├── SM_Optic_ACOG.uasset
│   │   │   ├── BP_Optic_RedDot.uasset
│   │   │   └── BP_Optic_ACOG.uasset
│   │   ├── Muzzle/
│   │   │   ├── SM_Muzzle_Suppressor_762.uasset
│   │   │   ├── SM_Muzzle_Suppressor_9mm.uasset
│   │   │   ├── SM_Muzzle_Compensator.uasset
│   │   │   └── BP_Muzzle_Suppressor.uasset
│   │   ├── Grips/
│   │   ├── Stocks/
│   │   └── Magazines/
│   │
│   └── Throwables/
│       ├── BP_Grenade_Frag.uasset
│       ├── BP_Grenade_Smoke.uasset
│       └── BP_Grenade_Flash.uasset
│
├── Maps/
│   │
│   ├── Raid_Industrial/                       ← Mapa "Planta Industrial Abandonada"
│   │   ├── Raid_Industrial.umap               ← Nivel principal (persistent level)
│   │   ├── Sub_Industrial_Zone_A.umap         ← Sublevel: zona norte (streaming)
│   │   ├── Sub_Industrial_Zone_B.umap         ← Sublevel: zona sur
│   │   ├── Sub_Industrial_Zone_C.umap         ← Sublevel: zona central (bunker)
│   │   ├── Lighting/
│   │   │   ├── LightingData_Day.uasset
│   │   │   └── LightingData_Night.uasset
│   │   └── NavMesh/
│   │       └── RecastNavMesh.uasset
│   │
│   ├── Raid_Urban/                            ← Mapa "Ciudad Abandonada"
│   │   └── (misma estructura que Raid_Industrial)
│   │
│   ├── MainMenu/
│   │   └── MainMenu.umap
│   │
│   ├── Loading/
│   │   └── LoadingScreen.umap
│   │
│   └── Shared/                                ← Assets de nivel compartidos entre mapas
│       ├── SpawnPoints/
│       │   └── BP_RaidSpawnPoint.uasset
│       ├── ExtractionZones/
│       │   ├── BP_ExtractionZone.uasset
│       │   └── BP_ExtractionZone_Vehicle.uasset
│       └── Boundaries/
│           └── BP_RaidBoundary.uasset         ← Límite invisible de la raid
│
├── Environment/
│   │
│   ├── Modular/
│   │   │
│   │   ├── Industrial/                        ← Kit modular para fábricas/almacenes
│   │   │   ├── Walls/
│   │   │   │   ├── SM_Ind_Wall_Straight_3m.uasset
│   │   │   │   ├── SM_Ind_Wall_Corner_90.uasset
│   │   │   │   ├── SM_Ind_Wall_Window_A.uasset
│   │   │   │   ├── SM_Ind_Wall_Door_Frame.uasset
│   │   │   │   └── SM_Ind_Wall_Damaged_A.uasset
│   │   │   ├── Floors/
│   │   │   │   ├── SM_Ind_Floor_Concrete.uasset
│   │   │   │   ├── SM_Ind_Floor_Metal_Grate.uasset
│   │   │   │   └── SM_Ind_Floor_Damaged.uasset
│   │   │   ├── Ceilings/
│   │   │   │   ├── SM_Ind_Ceiling_Flat.uasset
│   │   │   │   └── SM_Ind_Ceiling_Beams.uasset
│   │   │   ├── Pillars/
│   │   │   │   └── SM_Ind_Pillar_Square_3m.uasset
│   │   │   ├── Stairs/
│   │   │   │   └── SM_Ind_Stairs_Straight.uasset
│   │   │   └── Props/
│   │   │       ├── SM_Ind_Crate_Wood_A.uasset
│   │   │       ├── SM_Ind_Barrel_Metal.uasset
│   │   │       ├── SM_Ind_MachinePart_A.uasset
│   │   │       └── SM_Ind_Shelf_Metal.uasset
│   │   │
│   │   ├── Urban/                             ← Kit modular para ciudad/edificios
│   │   │   ├── Walls/
│   │   │   ├── Floors/
│   │   │   ├── Facades/
│   │   │   ├── Interiors/
│   │   │   └── Props/
│   │   │
│   │   ├── Military/                          ← Kit modular para instalaciones militares
│   │   │   ├── Walls/
│   │   │   ├── Barriers/
│   │   │   │   ├── SM_Mil_Sandbag_Single.uasset
│   │   │   │   ├── SM_Mil_Sandbag_Stack.uasset
│   │   │   │   ├── SM_Mil_Barrier_Concrete.uasset
│   │   │   │   └── SM_Mil_Barbed_Wire.uasset
│   │   │   └── Props/
│   │   │       ├── SM_Mil_Locker_Metal.uasset
│   │   │       └── SM_Mil_Ammo_Box.uasset
│   │   │
│   │   └── Shared/                            ← Piezas modulares reutilizables
│   │       ├── Doors/
│   │       │   ├── BP_Door_Wood.uasset        ← Puerta interactiva (Blueprint)
│   │       │   ├── BP_Door_Metal.uasset
│   │       │   └── BP_Door_Metal_Locked.uasset
│   │       ├── Windows/
│   │       └── Ladders/
│   │           └── BP_Ladder_Climbable.uasset ← Escala interactiva
│   │
│   ├── Terrain/
│   │   ├── Heightmaps/                        ← Mapas de altura para Landscape
│   │   ├── Layers/                            ← Capas de material del terreno
│   │   │   ├── M_Terrain_Industrial.uasset
│   │   │   └── M_Terrain_Urban.uasset
│   │   └── Foliage/
│   │       ├── FT_Grass_Dead.uasset           ← Foliage Types
│   │       ├── FT_Tree_Birch.uasset
│   │       └── FT_Rubble_Small.uasset
│   │
│   ├── Decals/
│   │   ├── Impacts/                           ← Marcas de bala, quemaduras
│   │   │   ├── M_Decal_BulletHole_Concrete.uasset
│   │   │   ├── M_Decal_BulletHole_Wood.uasset
│   │   │   └── M_Decal_BulletHole_Metal.uasset
│   │   ├── Blood/
│   │   │   ├── M_Decal_Blood_Pool.uasset
│   │   │   └── M_Decal_Blood_Splatter.uasset
│   │   └── Environment/
│   │       ├── M_Decal_Dirt.uasset
│   │       └── M_Decal_Graffiti_A.uasset
│   │
│   └── Sky/
│       ├── BP_DynamicSky.uasset               ← Sistema de cielo dinámico
│       ├── BP_WeatherSystem.uasset            ← Lluvia, niebla, viento
│       └── Textures/
│           └── T_Skybox_Industrial_Overcast.uasset
│
├── Gameplay/
│   │
│   ├── AI/
│   │   ├── BehaviorTrees/
│   │   │   ├── BT_PMC_Standard.uasset
│   │   │   ├── BT_PMC_Alert.uasset
│   │   │   ├── BT_PMC_Patrol.uasset
│   │   │   ├── BT_Mutant_Wander.uasset
│   │   │   └── BT_Boss_Barkov.uasset
│   │   ├── Blackboards/
│   │   │   ├── BB_Enemy_Base.uasset
│   │   │   └── BB_Boss_Extended.uasset
│   │   ├── Tasks/                             ← BT Task nodes custom
│   │   │   ├── BTT_TakeCoverNearby.uasset
│   │   │   ├── BTT_ThrowGrenade.uasset
│   │   │   ├── BTT_CallForBackup.uasset
│   │   │   └── BTT_ReloadWeapon.uasset
│   │   ├── Services/
│   │   │   ├── BTS_UpdateEnemyLocation.uasset
│   │   │   └── BTS_CheckForThreats.uasset
│   │   ├── Decorators/
│   │   │   ├── BTD_CanSeeTarget.uasset
│   │   │   ├── BTD_HasAmmo.uasset
│   │   │   └── BTD_IsInCoverRange.uasset
│   │   ├── Controllers/
│   │   │   ├── BP_AIController_PMC.uasset
│   │   │   ├── BP_AIController_Mutant.uasset
│   │   │   └── BP_AIController_Boss.uasset
│   │   └── EnvQuery/                          ← Environment Query System (EQS)
│   │       ├── EQS_FindCoverPoint.uasset
│   │       ├── EQS_FindPatrolPoint.uasset
│   │       └── EQS_FindFlankPosition.uasset
│   │
│   ├── Abilities/                             ← Gameplay Ability System (GAS)
│   │   ├── Attributes/
│   │   │   ├── GAS_AttributeSet_Health.uasset
│   │   │   └── GAS_AttributeSet_Stamina.uasset
│   │   ├── Effects/
│   │   │   ├── GE_Bleeding.uasset
│   │   │   ├── GE_Fracture.uasset
│   │   │   ├── GE_Adrenaline.uasset
│   │   │   └── GE_Suppression.uasset
│   │   └── Cues/
│   │       ├── GC_BloodSplatter.uasset
│   │       └── GC_ImpactSpark.uasset
│   │
│   ├── Items/
│   │   ├── BP_WorldItem.uasset                ← Item en el suelo (pickable)
│   │   ├── BP_LootContainer.uasset            ← Caja/taquilla de loot
│   │   ├── BP_LootContainer_Locked.uasset
│   │   └── BP_LootContainer_Boss.uasset       ← Garantiza drops raros
│   │
│   └── Volumes/
│       ├── BP_RadiationZone.uasset            ← Zona de daño continuo (zona caliente)
│       ├── BP_AudioZone.uasset                ← Trigger de audio ambiental
│       └── BP_ExclusionZone.uasset            ← Mata jugadores fuera de bounds
│
├── UI/
│   │
│   ├── HUD/
│   │   ├── WBP_HUD_Raid.uasset                ← Widget principal del HUD
│   │   ├── WBP_HUD_WeaponInfo.uasset
│   │   ├── WBP_HUD_HealthStatus.uasset        ← Silueta corporal
│   │   ├── WBP_HUD_StaminaBar.uasset
│   │   ├── WBP_HUD_MiniMap.uasset
│   │   ├── WBP_HUD_RaidTimer.uasset
│   │   ├── WBP_HUD_Crosshair.uasset
│   │   ├── WBP_HUD_HitMarker.uasset
│   │   ├── WBP_HUD_DamageDirection.uasset     ← Indicador de dirección del daño
│   │   └── WBP_HUD_Notifications.uasset       ← Notificaciones de misión, kills
│   │
│   ├── Inventory/
│   │   ├── WBP_Inventory_Main.uasset          ← Pantalla completa de inventario
│   │   ├── WBP_Inventory_Grid.uasset          ← Componente grid reutilizable
│   │   ├── WBP_Inventory_ItemSlot.uasset      ← Celda individual del grid
│   │   ├── WBP_Inventory_Tooltip.uasset       ← Tooltip con stats del item
│   │   ├── WBP_Inventory_ContextMenu.uasset   ← Menú click derecho
│   │   └── WBP_Inventory_ItemInspect.uasset   ← Vista 3D del item
│   │
│   ├── Map/
│   │   ├── WBP_Map_Full.uasset                ← Mapa desplegable (tecla M)
│   │   └── WBP_Map_ExtractionMarker.uasset
│   │
│   ├── Trader/
│   │   ├── WBP_Trader_Main.uasset
│   │   ├── WBP_Trader_ItemList.uasset
│   │   └── WBP_Trader_Transaction.uasset
│   │
│   ├── MainMenu/
│   │   ├── WBP_MainMenu.uasset
│   │   ├── WBP_MainMenu_LoadoutPrep.uasset    ← Preparación antes de raid
│   │   ├── WBP_MainMenu_Stash.uasset          ← Gestión del stash personal
│   │   └── WBP_MainMenu_Settings.uasset
│   │
│   └── Shared/
│       ├── WBP_Button_Primary.uasset          ← Botón reutilizable (estilo primario)
│       ├── WBP_Button_Secondary.uasset
│       ├── WBP_Icon_Item.uasset               ← Icono de item (para inventario/trader)
│       ├── WBP_ProgressBar_Custom.uasset
│       └── WBP_Popup_Confirm.uasset           ← Popup de confirmación
│
├── Audio/
│   │
│   ├── Weapons/
│   │   ├── Rifles/
│   │   │   ├── A_AK103_Fire_Close.uasset
│   │   │   ├── A_AK103_Fire_Far.uasset
│   │   │   ├── A_AK103_Fire_Suppressed.uasset
│   │   │   ├── A_AK103_Reload_In.uasset
│   │   │   ├── A_AK103_Reload_Out.uasset
│   │   │   └── A_AK103_BoltAction.uasset
│   │   ├── SMGs/ (misma estructura)
│   │   ├── Snipers/
│   │   ├── Shotguns/
│   │   ├── Pistols/
│   │   └── Shared/
│   │       ├── A_BulletWhiz_Close.uasset      ← Silbido de bala al pasar cerca
│   │       ├── A_BulletWhiz_Far.uasset
│   │       └── A_BulletImpact_Concrete.uasset
│   │
│   ├── Characters/
│   │   ├── Footsteps/
│   │   │   ├── A_Footstep_Concrete_Walk.uasset
│   │   │   ├── A_Footstep_Metal_Run.uasset
│   │   │   └── A_Footstep_Gravel_Jog.uasset
│   │   ├── Voice/
│   │   │   ├── A_PMC_Alert_Call.uasset
│   │   │   └── A_PMC_Death.uasset
│   │   └── Breathing/
│   │       ├── A_Player_Breathing_Normal.uasset
│   │       └── A_Player_Breathing_Exhausted.uasset
│   │
│   ├── Environment/
│   │   ├── Ambience/
│   │   │   ├── A_Amb_Industrial_Interior.uasset
│   │   │   ├── A_Amb_Wind_Outdoor.uasset
│   │   │   └── A_Amb_Rain_Heavy.uasset
│   │   └── Events/
│   │       └── A_Env_MetalCreak.uasset
│   │
│   ├── Music/
│   │   ├── MUS_MainMenu_Theme.uasset
│   │   ├── MUS_Raid_Insertion.uasset
│   │   ├── MUS_Raid_Tension_Loop.uasset
│   │   ├── MUS_Extraction_Success.uasset
│   │   └── MUS_Death_Sting.uasset
│   │
│   └── MetaSounds/
│       ├── MS_Weapon_Fire_Procedural.uasset   ← Grafo de disparo procedural
│       ├── MS_Footsteps_Surface_Adaptive.uasset ← Pasos que cambian según superficie
│       └── MS_Ambience_Dynamic.uasset          ← Ambiente dinámico por zona
│
├── VFX/
│   ├── Weapons/
│   │   ├── NS_MuzzleFlash_Rifle.uasset        ← Niagara System: fogonazo
│   │   ├── NS_BrassEjection.uasset            ← Niagara: expulsión de casquillo
│   │   ├── NS_TracerRound.uasset              ← Niagara: trazador (cada 5 balas)
│   │   └── NS_Suppressor_Smoke.uasset
│   ├── Impacts/
│   │   ├── NS_Impact_Concrete.uasset
│   │   ├── NS_Impact_Metal.uasset
│   │   ├── NS_Impact_Wood.uasset
│   │   ├── NS_Impact_Dirt.uasset
│   │   └── NS_Impact_Water.uasset
│   ├── Blood/
│   │   ├── NS_BloodSplatter_Hit.uasset
│   │   └── NS_BloodDrip_Loop.uasset
│   ├── Explosions/
│   │   ├── NS_Explosion_Grenade.uasset
│   │   └── NS_Explosion_Vehicle.uasset
│   └── Environment/
│       ├── NS_Smoke_Industrial.uasset
│       ├── NS_Dust_Footstep.uasset
│       └── NS_Rain_Drops.uasset
│
├── Data/
│   ├── DataTables/
│   │   ├── DT_ItemDefinitions.uasset          ← Todos los ítems del juego
│   │   ├── DT_WeaponStats.uasset              ← Stats base de cada arma
│   │   ├── DT_AmmoTypes.uasset                ← Tipos de munición y sus propiedades
│   │   ├── DT_ArmorTiers.uasset               ← Niveles de armadura
│   │   ├── DT_MedicalItems.uasset             ← Items médicos y sus efectos
│   │   ├── DT_LootTables.uasset               ← Qué puede aparecer en cada contenedor
│   │   ├── DT_TraderInventory.uasset          ← Inventario de cada trader por nivel
│   │   ├── DT_MissionDefinitions.uasset       ← Definición de misiones
│   │   ├── DT_SurfaceImpacts.uasset           ← Qué VFX/sonido poner en cada superficie
│   │   └── DT_BodyZoneMultipliers.uasset      ← Multiplicadores de daño por zona corporal
│   │
│   ├── DataAssets/
│   │   ├── Weapons/
│   │   │   ├── DA_Weapon_AK103.uasset
│   │   │   ├── DA_Weapon_M4A1.uasset
│   │   │   └── DA_Weapon_MP5.uasset
│   │   ├── Attachments/
│   │   │   ├── DA_Att_Suppressor_762.uasset
│   │   │   └── DA_Att_ACOG.uasset
│   │   └── Items/
│   │       ├── DA_Item_MedKit_Small.uasset
│   │       └── DA_Item_Bandage.uasset
│   │
│   └── Config/
│       ├── DA_GameBalance_Config.uasset        ← Variables de balance (ajustables sin código)
│       ├── DA_ServerConfig.uasset             ← Config del servidor
│       └── DA_EconomyConfig.uasset            ← Config de economía y traders
│
├── Input/
│   ├── IMC_OnFoot.uasset                      ← Input Mapping Context: a pie
│   ├── IMC_InVehicle.uasset                   ← Input Mapping Context: en vehículo
│   ├── IMC_UI.uasset                          ← Input Mapping Context: menús
│   ├── IA_Move.uasset                         ← Input Action: movimiento
│   ├── IA_Look.uasset
│   ├── IA_Sprint.uasset
│   ├── IA_Crouch.uasset
│   ├── IA_Jump.uasset
│   ├── IA_Fire.uasset
│   ├── IA_ADS.uasset
│   ├── IA_Reload.uasset
│   ├── IA_Interact.uasset
│   ├── IA_OpenInventory.uasset
│   ├── IA_OpenMap.uasset
│   ├── IA_LeanLeft.uasset
│   ├── IA_LeanRight.uasset
│   ├── IA_SwitchFireMode.uasset
│   └── IA_ThrowGrenade.uasset
│
└── _Dev/                                      ← ¡NO incluir en builds de producción!
    ├── _TestMaps/
    │   ├── TestMap_WeaponRange.umap
    │   ├── TestMap_BallisticsGun.umap
    │   └── TestMap_AIBehavior.umap
    ├── _Placeholder/                          ← Meshes grises de placeholder
    └── [Dev_NombreDesarrollador]/             ← Sandbox personal por dev
```

---

## 2. CONVENCIONES DE PREFIJOS DE ASSETS

| Tipo de Asset | Prefijo | Ejemplo |
|---------------|---------|---------|
| Blueprint | `BP_` | `BP_PlayerCharacter` |
| Skeletal Mesh | `SK_` | `SK_AK103` |
| Static Mesh | `SM_` | `SM_Ind_Wall_Straight` |
| Material | `M_` | `M_Weapon_Master` |
| Material Instance | `MI_` | `MI_AK103_Default` |
| Material Function | `MF_` | `MF_Blend_Detail_Normal` |
| Texture | `T_` | `T_AK103_Albedo` |
| Widget Blueprint | `WBP_` | `WBP_HUD_Raid` |
| AnimBP | `ABP_` | `ABP_Player_TPP` |
| Animation Montage | `AM_` | `AM_AK103_Fire_Hip` |
| Blend Space | `BS_` | `BS_Locomotion_Walk` |
| Aim Offset | `AO_` | `AO_Rifle_Hip` |
| DataTable | `DT_` | `DT_ItemDefinitions` |
| DataAsset | `DA_` | `DA_Weapon_AK103` |
| Niagara System | `NS_` | `NS_MuzzleFlash_Rifle` |
| Sound | `A_` | `A_AK103_Fire_Close` |
| MetaSound | `MS_` | `MS_Weapon_Fire_Procedural` |
| Music | `MUS_` | `MUS_MainMenu_Theme` |
| Behavior Tree | `BT_` | `BT_PMC_Standard` |
| Blackboard | `BB_` | `BB_Enemy_Base` |
| BT Task | `BTT_` | `BTT_TakeCoverNearby` |
| BT Service | `BTS_` | `BTS_UpdateEnemyLocation` |
| BT Decorator | `BTD_` | `BTD_CanSeeTarget` |
| EQS Query | `EQS_` | `EQS_FindCoverPoint` |
| Gameplay Effect | `GE_` | `GE_Bleeding` |
| Gameplay Cue | `GC_` | `GC_BloodSplatter` |
| Level Map | sin prefijo | `Raid_Industrial` |
| Foliage Type | `FT_` | `FT_Grass_Dead` |
| Curve | `CRV_` | `CRV_DamageFalloff` |

---

## 3. REGLAS DE GESTIÓN DE ASSETS

1. **Nunca poner assets en la raíz de Content/:** Siempre dentro de una subcarpeta organizada.
2. **Carpeta `_Dev/` nunca en producción:** Usar macro de cocina para excluirla del build final.
3. **Un folder por weapon, un folder por character:** No mezclar assets de distintos personajes/armas.
4. **Los Blueprints hijos junto al padre:** Si `BP_PMC_Elite` hereda de `BP_PMC_Base`, ambos van en `Enemies/PMC/Blueprints/`.
5. **Assets compartidos en `/Shared/`:** Si más de un sistema usa el mismo asset, moverlo a su correspondiente carpeta `Shared/`.
6. **Prefijos obligatorios:** Sin prefijo → el asset puede ser confundido con otro tipo, lo que causa errores de referencia.
7. **No renombrar sin actualizar referencias:** Usar `Asset Actions → Fix Up Redirectors` tras renombrar para evitar referencias rotas.
