# GUÍA DE NETWORKING MULTIJUGADOR EN UE5
## Proyecto: ZONA ROJA

**Versión:** 1.0  
**Motor:** Unreal Engine 5.3+  
**Arquitectura de red:** Servidor dedicado autoritativo  
**Backend de sesiones:** Steam / Epic Online Services (EOS)  

---

## 1. GESTIÓN DE SESIONES

### 1.1 Visión General de la Arquitectura

```
[Cliente]  ←── matchmaking ──► [Servicio de Matchmaking (EOS/Steam)]
                                          │
                                          │ asigna servidor
                                          ▼
[Cliente] ←─── conexión directa ──────► [Servidor Dedicado UE5]
                                          │
                                          │ autenticación y estado de cuenta
                                          ▼
                                   [Backend de Juego]
                                   (Node.js/Go/REST API)
                                          │
                                          ▼
                                   [Base de Datos]
                                   (stash, inventario, progresión del jugador)
```

### 1.2 Integración con Epic Online Services (EOS)

EOS es el sistema recomendado para UE5 por integración nativa. Proporciona:
- Autenticación multiplataforma (Steam, Epic, Xbox, PlayStation)
- Matchmaking y lobbies
- Anticheat integrado (EAC — Easy Anti-Cheat)
- Voice chat de proximidad
- Estadísticas y logros

**Configuración en UE5:**
```ini
; Config/DefaultEngine.ini
[OnlineSubsystem]
DefaultPlatformService=EOS

[OnlineSubsystemEOS]
bEnabled=true
ProductId=YOUR_PRODUCT_ID
SandboxId=YOUR_SANDBOX_ID
DeploymentId=YOUR_DEPLOYMENT_ID
ClientCredentialsId=YOUR_CLIENT_ID
ClientCredentialsSecret=YOUR_CLIENT_SECRET
bUseEAS=true   ; Epic Authentication Service
bUseEOSConnect=true
bMirrorStatsToEOS=true

[/Script/OnlineSubsystemUtils.IpNetDriver]
MaxClientRate=50000
MaxInternetClientRate=50000
```

### 1.3 Integración con Steamworks (alternativa/complementaria)

Si se usa Steam como plataforma principal:
```ini
[OnlineSubsystem]
DefaultPlatformService=Steam

[OnlineSubsystemSteam]
bEnabled=true
SteamDevAppId=480   ; Dev AppID, reemplazar con el AppID real
GameServerQueryPort=27015
bInitServerOnClient=false
bUseSteamNetworking=true  ; Steam relay para NAT traversal
```

### 1.4 Flujo de Creación y Unión a Raid

```cpp
// 1. El jugador selecciona el mapa y prepara su loadout

// 2. Solicitar matchmaking (en GameInstance o DedicatedLobbyManager)
void UZonaRojaGameInstance::FindOrCreateRaid(const FRaidSearchParams& Params)
{
    IOnlineSessionPtr SessionInterface = Online::GetSessionInterface(GetWorld());
    
    TSharedRef<FOnlineSessionSearch> SearchSettings = MakeShared<FOnlineSessionSearch>();
    SearchSettings->MaxSearchResults = 20;
    SearchSettings->bIsLanQuery = false;
    SearchSettings->QuerySettings.Set(
        FName("RAID_MAP"),
        Params.MapName,
        EOnlineComparisonOp::Equals
    );
    SearchSettings->QuerySettings.Set(
        FName("RAID_MODE"),
        FString("standard"),
        EOnlineComparisonOp::Equals
    );
    
    SessionInterface->FindSessions(0, SearchSettings);
}

// 3. Si no hay sesión disponible, crear una nueva
void UZonaRojaGameInstance::CreateRaidSession(const FRaidSessionConfig& Config)
{
    FOnlineSessionSettings SessionSettings;
    SessionSettings.NumPublicConnections = Config.MaxPlayers; // 6-12
    SessionSettings.bIsLANMatch = false;
    SessionSettings.bUsesPresence = false; // PvPvE, no mostrar en "jugando con amigos"
    SessionSettings.bShouldAdvertise = true;
    SessionSettings.bAllowJoinInProgress = false; // No unirse a raid en curso
    SessionSettings.Set(FName("RAID_MAP"), Config.MapName, EOnlineDataAdvertisementType::ViaOnlineService);
    SessionSettings.Set(FName("RAID_DURATION"), Config.DurationSeconds, EOnlineDataAdvertisementType::ViaOnlineService);
    
    SessionInterface->CreateSession(0, NAME_GameSession, SessionSettings);
}
```

---

## 2. RECOMENDACIONES DE JUGADORES POR RAID

### 2.1 Balance de Jugadores

| Tamaño del mapa | Jugadores recomendados | Mínimo | Máximo |
|-----------------|----------------------|--------|--------|
| Pequeño (< 500m²) | 4-6 | 2 | 8 |
| Mediano (500-1500m²) | 6-10 | 4 | 12 |
| Grande (> 1500m²) | 8-12 | 6 | 14 |

**Justificación de 6-12 jugadores como rango óptimo:**
- Menos de 6: La raid se vuelve demasiado solitaria, poca tensión PvP
- 7-10: El rango ideal para equilibrio entre encuentros PvP y exploración segura
- Más de 12: Demasiado caótico, el mapa se satura y la extracción se vuelve imposible

### 2.2 Modos de Raid

| Modo | Jugadores | Equipos |
|------|-----------|---------|
| Solo | 1-12 | Todos contra todos |
| Duo | 2 por equipo, 4-12 jugadores total | Por equipo de 2 |
| Squad | 3-4 por equipo, 6-12 jugadores total | Por equipo |
| PvE (sin PvP) | 1-4 | Cooperativo, sin daño entre equipos |

### 2.3 Configuración de Red por Número de Jugadores

```cpp
// En la configuración del Game Server:
// Con 12 jugadores + IA (30-50 bots activos), el servidor necesita:
// - Bandwidth: ~50-80 KB/s por jugador (full combat)
// - Tick Rate servidor: 60 Hz para combate suave
// - Tick Rate replicación: 30 Hz para actors no-jugador

// En DefaultEngine.ini para servidor dedicado:
[/Script/Engine.Engine]
NetServerMaxTickRate=60
```

---

## 3. COMPENSACIÓN DE LAG

### 3.1 El Problema del Lag en Shooters

Cuando el cliente dispara, lo hace sobre su representación local del mundo (que está N milisegundos en el pasado respecto al servidor). Sin compensación de lag, el servidor rechazaría hits válidos porque el target ya se movió.

### 3.2 Lag Compensation para Hitscan

```cpp
// Sistema de Lag Compensation — almacena historial de posiciones
class ULagCompensationComponent : public UActorComponent
{
    // Historial de posiciones de todos los jugadores (circular buffer)
    struct FPositionSnapshot
    {
        float Timestamp;
        TMap<APlayerCharacter*, FTransform> PlayerTransforms;
        TMap<APlayerCharacter*, TArray<FTransform>> BoneTransforms; // Para hitboxes corporales
    };

    TArray<FPositionSnapshot> PositionHistory;
    const int32 MaxHistoryFrames = 30; // ~0.5s a 60Hz
    const float MaxLagCompensationTime = 0.25f; // 250ms máximo (ajustable)

    // Guardar snapshot cada frame en el servidor
    void RecordSnapshot();

    // Rewind del mundo al timestamp del cliente
    FPositionSnapshot* GetSnapshotAtTime(float Timestamp);

    // Restaurar posiciones, hacer el trace, volver al presente
    bool PerformLagCompensatedHitcheck(
        const FHitConfirmData& HitData,
        APlayerController* ShootingPlayer
    );
};
```

**Flujo de validación server-side:**

```cpp
bool ULagCompensationComponent::PerformLagCompensatedHitcheck(
    const FHitConfirmData& HitData,
    APlayerController* ShootingPlayer)
{
    float ClientTimestamp = HitData.ClientTimestamp;
    float ServerTime = GetWorld()->GetTimeSeconds();
    float Ping = ShootingPlayer->PlayerState->GetPingInMilliseconds() / 1000.f;
    
    // Validar que el timestamp no es demasiado antiguo
    float TimeDiff = ServerTime - ClientTimestamp;
    if (TimeDiff > MaxLagCompensationTime + 0.05f) // 50ms de tolerancia extra
    {
        // Timestamp demasiado antiguo, podría ser intento de exploit
        UE_LOG(LogZonaRoja, Warning, TEXT("Hit rejected: timestamp too old (%.3f seconds)"), TimeDiff);
        return false;
    }
    
    // Encontrar snapshot más cercano al timestamp del cliente
    FPositionSnapshot* Snapshot = GetSnapshotAtTime(ClientTimestamp);
    if (!Snapshot) return false;
    
    // Mover temporalmente todos los jugadores a sus posiciones pasadas
    TMap<APlayerCharacter*, FTransform> OriginalTransforms;
    for (auto& [Player, PastTransform] : Snapshot->PlayerTransforms)
    {
        OriginalTransforms.Add(Player, Player->GetActorTransform());
        Player->SetActorTransform(PastTransform);
        // Mover hitboxes de huesos también
        if (Snapshot->BoneTransforms.Contains(Player))
        {
            Player->GetMesh()->SetBoneTransformsByName(Snapshot->BoneTransforms[Player]);
        }
    }
    
    // Realizar el raytrace en el mundo "rebobinado"
    FHitResult ServerHitResult;
    FVector TraceStart = HitData.MuzzlePosition;
    FVector TraceEnd = TraceStart + HitData.ShootDirection * HitData.WeaponRange;
    
    bool bHit = GetWorld()->LineTraceSingleByChannel(
        ServerHitResult,
        TraceStart, TraceEnd,
        ECC_Bullet
    );
    
    // Restaurar posiciones actuales
    for (auto& [Player, OrigTransform] : OriginalTransforms)
    {
        Player->SetActorTransform(OrigTransform);
    }
    
    if (!bHit) return false;
    
    // Verificar que el actor golpeado coincide con el reportado por el cliente
    APlayerCharacter* HitCharacter = Cast<APlayerCharacter>(ServerHitResult.GetActor());
    if (!HitCharacter || HitCharacter != HitData.ReportedTarget) return false;
    
    // Verificar distancia de impacto vs reportada (tolerancia 2%)
    float DistanceDiff = FVector::Dist(ServerHitResult.Location, HitData.HitLocation);
    if (DistanceDiff > HitData.WeaponRange * 0.02f) return false;
    
    // Hit válido — aplicar daño en el servidor
    ApplyDamageToCharacter(HitCharacter, HitData);
    return true;
}
```

### 3.3 Lag Compensation para Proyectiles Físicos

Para proyectiles físicos (francotiradores, lanzagranadas), la compensación de lag se gestiona de forma diferente:

```cpp
// Los proyectiles físicos son simulados en el servidor (authoritative)
// Los clientes solo ven una predicción visual

// Spawneo de proyectil (solo en servidor):
UFUNCTION(Server, Reliable)
void AFirearmBase::ServerSpawnProjectile(
    FVector MuzzleLocation, FVector LaunchVelocity, float ClientTimestamp)
{
    // Compensar la posición de spawn según el lag del cliente
    float ClientPing = GetInstigator()->GetPlayerState()->GetPingInMilliseconds() / 1000.f;
    float CompensationTime = FMath::Min(ClientPing, 0.15f); // máx 150ms compensación
    
    // Spawneo del proyectil
    AProjectileBase* Projectile = GetWorld()->SpawnActor<AProjectileBase>(
        WeaponData->ProjectileClass,
        MuzzleLocation, LaunchVelocity.Rotation()
    );
    
    if (Projectile)
    {
        Projectile->LaunchWithCompensation(LaunchVelocity, CompensationTime);
    }
}
```

### 3.4 Configuración del Tick Rate del Servidor

El tick rate del servidor es crítico para la calidad del lag compensation:

```ini
; DefaultEngine.ini
[/Script/Engine.Engine]
NetServerMaxTickRate=60      ; Hz del servidor (60 ideal, 30 mínimo aceptable)
FixedFrameRate=60.0          ; Frame rate fijo del servidor dedicado

[/Script/OnlineSubsystemUtils.IpNetDriver]
MaxClientRate=50000          ; Bits por segundo por cliente (saliente del servidor)
MaxInternetClientRate=50000
```

| Tick Rate | Calidad | CPU Server | Recomendado |
|-----------|---------|------------|-------------|
| 30 Hz | Aceptable | Bajo | Budget servers |
| 60 Hz | Buena | Medio | Producción estándar |
| 128 Hz | Excelente | Alto | Servidores premium |

---

## 4. CONSIDERACIONES ANTI-CHEAT

### 4.1 Principios de Autoridad del Servidor

**Regla de oro: El servidor NUNCA confía en el cliente para datos críticos.**

| Dato | Autoridad | Por qué |
|------|-----------|---------|
| Posición del jugador | Servidor (con predicción cliente) | Evitar speedhack/teleport |
| HP y daño | Servidor exclusivo | Evitar god mode |
| Contenido del inventario | Servidor exclusivo | Evitar item duplication |
| Resultado de hits | Servidor (con lag comp) | Evitar aimbot advantage |
| Timer de extracción | Servidor exclusivo | Evitar extracción instantánea |
| Visibilidad de enemigos | Servidor (no enviar posición a clientes que no deberían verla) | Evitar wallhacks via packet sniffing |
| Economía (dinero, items) | Backend externo | Evitar modificación de memoria |

### 4.2 Visibility Culling (Anti-Wallhack por Protocol)

No enviar información de actores que el jugador no puede ver:

```cpp
// Sobrescribir IsNetRelevantFor en APlayerCharacter:
bool APlayerCharacter::IsNetRelevantFor(
    const AActor* RealViewer, const AActor* ViewTarget, const FVector& SrcLocation) const
{
    // Siempre relevante para el propio jugador
    if (RealViewer == this) return true;
    
    // Verificar distancia
    float Distance = FVector::Dist(GetActorLocation(), SrcLocation);
    if (Distance > NetCullDistanceSquared) return false;
    
    // Verificación de línea de visión (oclusión)
    // Solo enviar replicación si el jugador está potencialmente visible
    FHitResult HitResult;
    FCollisionQueryParams Params;
    Params.AddIgnoredActor(this);
    Params.AddIgnoredActor(RealViewer->GetOwner());
    
    bool bHasLOS = !GetWorld()->LineTraceSingleByChannel(
        HitResult,
        SrcLocation,                    // Posición del observador
        GetActorLocation(),             // Posición del observado
        ECC_Visibility_Custom,          // Canal de visibilidad personalizado
        Params
    );
    
    // Margen adicional: si estuvo visible en los últimos 2 segundos, mantener relevancia
    // (evitar flickering de replicación en coberturas)
    float TimeSinceLastVisible = GetWorld()->GetTimeSeconds() - LastVisibleToPlayerTime;
    
    return bHasLOS || (TimeSinceLastVisible < 2.0f);
}
```

**Nota importante:** Este sistema reduce la información disponible para herramientas de wallhack que leen paquetes de red. Sin embargo, no es infalible contra cheats que operan a nivel de driver gráfico (overlay wallhacks que leen el frame buffer).

### 4.3 Easy Anti-Cheat (EAC) con EOS

Easy Anti-Cheat se integra con EOS y detecta:
- Modificación de memoria (inyección de DLLs)
- Speedhacks (detección de velocidad de reloj manipulada)
- Aim bots (patrones de movimiento de ratón imposibles)
- Wallhacks a nivel de renderer
- Modificación de archivos del juego

**Configuración en UE5:**
```ini
; Config/DefaultEngine.ini
[EasyAntiCheatSDK]
bEACEnabled=true
ProductId=YOUR_EAC_PRODUCT_ID
```

Requiere también configuración del lanzador del juego con el SDK de EAC.

### 4.4 Validaciones Server-Side Adicionales

```cpp
// Validar velocidad de movimiento (anti-speedhack)
void APlayerCharacter::ServerValidateMovement(FVector NewLocation, float DeltaTime)
{
    float MaxAllowedSpeed = GetCharacterMovement()->MaxWalkSpeed * 1.15f; // 15% tolerancia
    float ActualSpeed = FVector::Dist(GetActorLocation(), NewLocation) / DeltaTime;
    
    if (ActualSpeed > MaxAllowedSpeed * 100.f) // cm/s
    {
        // Teleport imposible detectado
        SetActorLocation(GetActorLocation()); // Revertir al servidor
        UE_LOG(LogZonaRoja, Warning, TEXT("Player %s: movement anomaly (%.1f cm/s)"),
            *GetPlayerState()->GetPlayerName(), ActualSpeed);
        // Reportar al sistema anti-cheat
    }
}

// Validar cadencia de disparo (anti-trigger bot / rapid fire)
void AFirearmBase::ServerCheckFireRate()
{
    float TimeSinceLastShot = GetWorld()->GetTimeSeconds() - LastShotTime;
    float MinTimeBetweenShots = 60.f / ComputedStats.FireRate * 0.85f; // 15% tolerancia
    
    if (TimeSinceLastShot < MinTimeBetweenShots)
    {
        // Disparo demasiado rápido, descartar
        return;
    }
    
    LastShotTime = GetWorld()->GetTimeSeconds();
    // Proceder con el disparo...
}
```

---

## 5. VALIDACIÓN SERVER-SIDE DE HITS

### 5.1 Estructura de Datos de Confirmación de Hit

```cpp
USTRUCT(BlueprintType)
struct FHitConfirmData
{
    GENERATED_BODY()

    UPROPERTY()
    AActor* ReportedTarget;         // Actor que el cliente dice haber golpeado

    UPROPERTY()
    EBodyPart ReportedHitZone;      // Zona corporal reportada

    UPROPERTY()
    FVector HitLocation;            // Posición del impacto en el mundo

    UPROPERTY()
    FVector MuzzlePosition;         // Posición de la boca del cañón al disparar

    UPROPERTY()
    FVector ShootDirection;         // Dirección normalizada del disparo

    UPROPERTY()
    float ClientTimestamp;          // GetWorld()->GetTimeSeconds() en el cliente

    UPROPERTY()
    FName WeaponItemID;             // ID del arma usada

    UPROPERTY()
    FName AmmoItemID;               // ID del tipo de munición

    UPROPERTY()
    float WeaponRange;              // Alcance máximo del arma (para limitar el trace)
};
```

### 5.2 Pipeline Completo de Validación

```
CLIENTE dispara:
  1. Registrar: MuzzlePosition, ShootDirection, ClientTimestamp
  2. Hacer LineTrace local
  3. Si hay hit: guardar HitResult en HitConfirmData
  4. Enviar ServerConfirmHit RPC con HitConfirmData

SERVIDOR recibe ServerConfirmHit:
  1. Validar timestamp (no demasiado antiguo)
  2. Validar que el cliente tiene un arma equipada del WeaponItemID indicado
  3. Validar que el arma tiene munición del tipo indicado
  4. Verificar cadencia (no dispara más rápido que el máximo del arma)
  5. Realizar lag compensation (rewind del mundo al timestamp)
  6. Realizar LineTrace server-side
  7. Comparar resultado con lo reportado por el cliente:
     - ¿Mismo actor golpeado? (±tolerancia si target se movió)
     - ¿Distancia de hit razonable? (±2% del alcance máximo)
     - ¿Ángulo de disparo válido? (±5 grados de desviación permitida)
  8. Si todo válido: ApplyDamage(target, calculatedDamage, hitZone)
  9. Consumir munición en el servidor
  10. Enviar feedback al cliente (HitConfirmed + daño aplicado)
  11. Si inválido: Log, potencialmente reportar al sistema anti-cheat
```

---

## 6. OPTIMIZACIONES DE RED

### 6.1 Network Relevancy y LOD de Red

```cpp
// Configurar niveles de actualización según distancia
void APlayerCharacter::PostInitializeComponents()
{
    Super::PostInitializeComponents();
    
    // Reducir frecuencia de replicación para jugadores lejanos
    // Esto reduce el bandwidth sin afectar la experiencia de combate cercano
    NetUpdateFrequency = 60.f;           // Hz máx para jugadores cercanos
    MinNetUpdateFrequency = 15.f;        // Hz mínimo para jugadores muy lejanos
    NetCullDistanceSquared = 25000.f * 25000.f; // 250m de distancia de culling
}

// Para bots IA: actualización menos frecuente
void AAICharacterBase::PostInitializeComponents()
{
    Super::PostInitializeComponents();
    NetUpdateFrequency = 30.f;
    MinNetUpdateFrequency = 5.f;
    NetCullDistanceSquared = 20000.f * 20000.f; // 200m
}
```

### 6.2 Compresión de Variables Replicadas

```cpp
// En lugar de replicar floats completos, usar versiones cuantizadas
// UE5 ofrece tipos de cuantización:

// Posición con precisión de 1cm (en lugar de float completo):
UPROPERTY(Replicated)
FVector_NetQuantize100 Position; // 1 décimo de centímetro de precisión

// Rotación con precisión de ~1.4 grados (suficiente para animaciones TPP):
UPROPERTY(Replicated)
FRotator_NetQuantize Rotation;

// HP como int en lugar de float (ahorra bytes):
UPROPERTY(Replicated)
uint8 HealthPercent; // 0-255 → mapear a 0-100%

// Velocidad de movimiento cuantizada:
UPROPERTY(Replicated)
FVector_NetQuantize10 Velocity;
```

### 6.3 Bandwidth por Jugador (Estimación)

| Estado del jugador | Datos/frame (60Hz) | KB/s |
|--------------------|-------------------|------|
| Quieto, sin combate | ~50 bytes | 3 KB/s |
| Moviéndose | ~120 bytes | 7 KB/s |
| En combate activo | ~300 bytes | 18 KB/s |
| Combate + efectos | ~500 bytes | 30 KB/s |

**Ancho de banda total del servidor (12 jugadores en combate activo):**
- Saliente: ~360 KB/s (12 × 30 KB/s)
- Entrante: ~60 KB/s (principalmente hit confirmations y inputs)
- **Total estimado: ~420 KB/s = ~3.4 Mbps** — muy manejable para servidores dedicados modernos

---

## 7. AUDIO DE PROXIMIDAD (VOICE CHAT)

### 7.1 Configuración de Voice Chat con EOS

```cpp
// En GameMode o PlayerController, configurar voice chat de proximidad
void ARaidPlayerController::SetupVoiceChat()
{
    // Solo comunicarse con compañeros de equipo (sin proximity para enemigos)
    // Excepto en modo Solo: sin voice chat entre jugadores enemigos por defecto
    
    IVoiceChatUser* VoiceChatUser = IVoiceChat::Get()->CreateUser();
    if (VoiceChatUser)
    {
        // Canal de equipo (siempre activo)
        VoiceChatUser->JoinChannel(
            FString::Printf(TEXT("team_%d"), TeamID),
            FString(),
            EVoiceChatChannelType::Team
        );
        
        // Canal de proximidad (voces cercanas de enemigos, opcional)
        // Requiere estar a < 10m de otro jugador para escucharse
        if (bProximityChatEnabled)
        {
            VoiceChatUser->JoinChannel(
                TEXT("world_proximity"),
                FString(),
                EVoiceChatChannelType::Positional
            );
        }
    }
}
```

---

## 8. RECUPERACIÓN DE DESCONEXIÓN

### 8.1 Manejo de Desconexiones Inesperadas

```
JUGADOR SE DESCONECTA:
  1. Servidor detecta pérdida de conexión
  2. Personaje permanece en el mundo 60 segundos
     - IA puede atacarlo
     - Otros jugadores pueden matarlo y lootearlo
  3. Si el jugador se reconecta antes de 60s: restablece sesión
  4. Si no se reconecta: personaje muere, pierde el equipo

RECONEXIÓN:
  1. Cliente solicita reconexión con el mismo SessionID
  2. Servidor verifica que la raid sigue activa y hay slot disponible
  3. Se restaura el estado del personaje (posición, HP, inventario)
  4. Tiempo de gracia: los primeros 5s tras reconectar, sin daño
```

```cpp
// En ARaidGameMode:
void ARaidGameMode::HandleDisconnect(UWorld* InWorld, UNetDriver* NetDriver)
{
    Super::HandleDisconnect(InWorld, NetDriver);
    
    // Encontrar el PlayerController desconectado
    // Iniciar timer de 60s antes de matar el personaje
    // Guardar estado del inventario en memoria (o base de datos) por si reconecta
}
```

---

*Este documento cubre la arquitectura de red principal. Para detalles de armas, ver WEAPONS_SYSTEM.md. Para arquitectura de gameplay general, ver UE5_ARCHITECTURE.md.*
