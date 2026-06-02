// PMCCharacter.cpp
// Implementación del personaje enemigo PMC
// Gestiona percepción, transiciones de estado, combate y muerte

#include "AI/PMCCharacter.h"
#include "AI/PMCAIController.h"
#include "AI/ZRCoverPoint.h"
#include "Components/HealthComponent.h"
#include "Characters/Player/ZRPlayerCharacter.h"
#include "Data/ZRTypes.h"

#include "Perception/AIPerceptionComponent.h"
#include "Perception/AISenseConfig_Sight.h"
#include "Perception/AISenseConfig_Hearing.h"
#include "Perception/AISense_Sight.h"
#include "Perception/AISense_Hearing.h"
#include "AIController.h"
#include "NavigationSystem.h"

#include "Components/CapsuleComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Net/UnrealNetwork.h"
#include "Kismet/GameplayStatics.h"
#include "DrawDebugHelpers.h"

APMCCharacter::APMCCharacter()
{
	// Habilitar tick para actualizar TimeSinceLastShot
	PrimaryActorTick.bCanEverTick = true;

	// Replicación habilitada para multijugador
	bReplicates = true;
	SetReplicateMovement(true);

	// ---- Componente de salud ----
	HealthComp = CreateDefaultSubobject<UHealthComponent>(TEXT("HealthComp"));

	// ---- Componente de percepción ----
	PerceptionComp = CreateDefaultSubobject<UAIPerceptionComponent>(TEXT("PerceptionComp"));

	// Configurar sensor de visión
	UAISenseConfig_Sight* SightConfig = CreateDefaultSubobject<UAISenseConfig_Sight>(TEXT("SightConfig"));
	SightConfig->SightRadius = 2000.0f;
	SightConfig->LoseSightRadius = SightConfig->SightRadius + 200.0f;
	SightConfig->PeripheralVisionAngleDegrees = 70.0f;
	SightConfig->SetMaxAge(10.0f);
	SightConfig->AutoSuccessRangeFromLastSeenLocation = 400.0f;
	SightConfig->DetectionByAffiliation.bDetectEnemies = true;
	SightConfig->DetectionByAffiliation.bDetectNeutrals = true;
	SightConfig->DetectionByAffiliation.bDetectFriendlies = false;
	PerceptionComp->ConfigureSense(*SightConfig);

	// Configurar sensor de audición
	UAISenseConfig_Hearing* HearingConfig = CreateDefaultSubobject<UAISenseConfig_Hearing>(TEXT("HearingConfig"));
	HearingConfig->HearingRange = 1500.0f;
	HearingConfig->SetMaxAge(5.0f);
	HearingConfig->DetectionByAffiliation.bDetectEnemies = true;
	HearingConfig->DetectionByAffiliation.bDetectNeutrals = true;
	HearingConfig->DetectionByAffiliation.bDetectFriendlies = false;
	PerceptionComp->ConfigureSense(*HearingConfig);

	PerceptionComp->SetDominantSense(SightConfig->GetSenseImplementation());

	// ---- Mesh del arma ----
	WeaponMesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("WeaponMesh"));
	WeaponMesh->SetupAttachment(GetMesh(), FName("hand_r_socket"));
	WeaponMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
}

void APMCCharacter::BeginPlay()
{
	Super::BeginPlay();

	// Aplicar configuración de estadísticas según el nivel asignado
	ApplyTierConfig();

	// Vincular el componente de percepción al callback
	if (PerceptionComp)
	{
		PerceptionComp->OnTargetPerceptionUpdated.AddDynamic(this, &APMCCharacter::OnPerceptionUpdated);
	}

	// Vincular los delegados del componente de salud
	if (HealthComp)
	{
		HealthComp->OnDeath.AddDynamic(this, &APMCCharacter::OnPMCDeath);
		HealthComp->OnDamageReceived.AddDynamic(this, &APMCCharacter::OnTakeDamage);
	}

	// Cachear referencia al AIController
	CachedAIController = Cast<APMCAIController>(GetController());
}

void APMCCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	// Acumular tiempo desde el último disparo (solo en servidor)
	if (HasAuthority())
	{
		TimeSinceLastShot += DeltaTime;
	}
}

void APMCCharacter::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	// Replicar estado actual a todos los clientes
	DOREPLIFETIME(APMCCharacter, CurrentState);

	// Replicar última posición conocida del jugador a todos los clientes
	DOREPLIFETIME(APMCCharacter, LastKnownPlayerLocation);
}

// ------------------------------------------------------------
// CONFIGURACION POR NIVEL
// ------------------------------------------------------------

void APMCCharacter::ApplyTierConfig()
{
	// Asignar estadísticas predefinidas según el nivel del PMC
	switch (Tier)
	{
	case EPMCTier::Novice:
		// Novato: parámetros básicos, sin capacidades tácticas avanzadas
		StatConfig.SightRadius         = 2000.0f;
		StatConfig.HearingRange        = 1500.0f;
		StatConfig.ReactionTime        = 0.8f;
		StatConfig.AccuracySpread      = 4.0f;
		StatConfig.FireRate            = 0.4f;
		StatConfig.PatrolSpeed         = 180.0f;
		StatConfig.AlertSpeed          = 300.0f;
		StatConfig.CombatSpeed         = 350.0f;
		StatConfig.MaxGroupCallRadius  = 2000;
		StatConfig.bCanFlank           = false;
		StatConfig.bCallsForBackup     = false;
		break;

	case EPMCTier::Veteran:
		// Veterano: más preciso y rápido, puede llamar refuerzos
		StatConfig.SightRadius         = 3000.0f;
		StatConfig.HearingRange        = 2000.0f;
		StatConfig.ReactionTime        = 0.4f;
		StatConfig.AccuracySpread      = 2.0f;
		StatConfig.FireRate            = 0.25f;
		StatConfig.PatrolSpeed         = 200.0f;
		StatConfig.AlertSpeed          = 350.0f;
		StatConfig.CombatSpeed         = 400.0f;
		StatConfig.MaxGroupCallRadius  = 3000;
		StatConfig.bCanFlank           = false;
		StatConfig.bCallsForBackup     = true;
		break;

	case EPMCTier::Elite:
		// Élite: máxima competencia táctica, puede flanquear y llama refuerzos
		StatConfig.SightRadius         = 4000.0f;
		StatConfig.HearingRange        = 2500.0f;
		StatConfig.ReactionTime        = 0.2f;
		StatConfig.AccuracySpread      = 0.8f;
		StatConfig.FireRate            = 0.15f;
		StatConfig.PatrolSpeed         = 220.0f;
		StatConfig.AlertSpeed          = 380.0f;
		StatConfig.CombatSpeed         = 450.0f;
		StatConfig.MaxGroupCallRadius  = 4000;
		StatConfig.bCanFlank           = true;
		StatConfig.bCallsForBackup     = true;
		break;
	}

	// Aplicar velocidades al CharacterMovement
	if (UCharacterMovementComponent* MoveComp = GetCharacterMovement())
	{
		MoveComp->MaxWalkSpeed = StatConfig.PatrolSpeed;
	}

	// Actualizar radios del sensor de visión con los valores del tier
	if (PerceptionComp)
	{
		// Actualizar configuración de visión en tiempo de ejecución si ya existe
		UAISenseConfig_Sight* SightConfig = Cast<UAISenseConfig_Sight>(
			PerceptionComp->GetSenseConfig(UAISense_Sight::GetSenseID<UAISense_Sight>()));
		if (SightConfig)
		{
			SightConfig->SightRadius = StatConfig.SightRadius;
			SightConfig->LoseSightRadius = StatConfig.SightRadius + 200.0f;
			PerceptionComp->RequestStimuliListenerUpdate();
		}
	}
}

// ------------------------------------------------------------
// MAQUINA DE ESTADOS
// ------------------------------------------------------------

void APMCCharacter::SetState(EPMCState NewState)
{
	// Solo el servidor cambia el estado autoritativo
	if (!HasAuthority())
	{
		return;
	}

	// Evitar transiciones al mismo estado (previene loops)
	if (CurrentState == NewState)
	{
		return;
	}

	// No transicionar desde muerto
	if (CurrentState == EPMCState::Dead)
	{
		return;
	}

	EPMCState OldState = CurrentState;
	CurrentState = NewState;

	// Actualizar velocidad de movimiento según el nuevo estado
	if (UCharacterMovementComponent* MoveComp = GetCharacterMovement())
	{
		switch (NewState)
		{
		case EPMCState::Patrol:
		case EPMCState::Suspicious:
			MoveComp->MaxWalkSpeed = StatConfig.PatrolSpeed;
			break;
		case EPMCState::Alert:
			MoveComp->MaxWalkSpeed = StatConfig.AlertSpeed;
			break;
		case EPMCState::Combat:
			MoveComp->MaxWalkSpeed = StatConfig.CombatSpeed;
			break;
		case EPMCState::Dead:
			MoveComp->MaxWalkSpeed = 0.0f;
			break;
		}
	}

	// Notificar al AIController para sincronizar el Blackboard
	if (!CachedAIController)
	{
		CachedAIController = Cast<APMCAIController>(GetController());
	}

	if (CachedAIController)
	{
		CachedAIController->UpdateBlackboardFromState(NewState);
	}

	// Actualizar el AnimBP en el servidor
	UpdateAnimBP();

	UE_LOG(LogTemp, Log, TEXT("[PMC] %s: Estado %d → %d"),
		*GetName(), (int32)OldState, (int32)NewState);
}

// ------------------------------------------------------------
// PERCEPCION
// ------------------------------------------------------------

void APMCCharacter::OnPerceptionUpdated(AActor* Actor, FAIStimulus Stimulus)
{
	// Solo procesar en el servidor
	if (!HasAuthority())
	{
		return;
	}

	// Ignorar actores que no sean el jugador
	AZRPlayerCharacter* Player = Cast<AZRPlayerCharacter>(Actor);
	if (!Player)
	{
		return;
	}

	// Ignorar jugadores muertos
	if (!Player->IsAlive())
	{
		return;
	}

	if (Stimulus.WasSuccessfullySensed())
	{
		// Estímulo detectado: actualizar posición conocida
		LastKnownPlayerLocation = Actor->GetActorLocation();

		// Actualizar el Blackboard del controlador
		if (!CachedAIController)
		{
			CachedAIController = Cast<APMCAIController>(GetController());
		}

		if (CachedAIController)
		{
			CachedAIController->SetLastKnownLocation(LastKnownPlayerLocation);
		}

		// Transicionar al estado apropiado según el tipo de estímulo
		if (Stimulus.Type == UAISense::GetSenseID<UAISense_Sight>())
		{
			// Estímulo visual: transicionar a combate
			if (CurrentState != EPMCState::Combat && CurrentState != EPMCState::Dead)
			{
				if (CachedAIController)
				{
					CachedAIController->SetTarget(Actor);
				}
				SetState(EPMCState::Combat);

				// Llamar refuerzos si el PMC tiene esa capacidad
				if (StatConfig.bCallsForBackup)
				{
					CallForBackup();
				}
			}
		}
		else if (Stimulus.Type == UAISense::GetSenseID<UAISense_Hearing>())
		{
			// Estímulo auditivo: ir a investigar si no está ya en combate
			if (CurrentState == EPMCState::Patrol || CurrentState == EPMCState::Suspicious)
			{
				SetState(EPMCState::Suspicious);
			}
		}
	}
	else
	{
		// Perdió el estímulo visual: si estaba en combate, pasar a alerta
		if (Stimulus.Type == UAISense::GetSenseID<UAISense_Sight>())
		{
			if (CurrentState == EPMCState::Combat)
			{
				SetState(EPMCState::Alert);
			}
		}
	}
}

// ------------------------------------------------------------
// DAÑO RECIBIDO
// ------------------------------------------------------------

void APMCCharacter::OnTakeDamage(float Damage, EBodyPart Part, EDamageType Type,
	AActor* DamageInstigator, const FHitResult& HitResult)
{
	// Solo procesar en el servidor
	if (!HasAuthority())
	{
		return;
	}

	// Si ya está muerto o procesando la muerte, ignorar
	if (bIsDeathProcessed || CurrentState == EPMCState::Dead)
	{
		return;
	}

	// Actualizar última posición conocida del atacante si existe
	if (DamageInstigator)
	{
		LastKnownPlayerLocation = DamageInstigator->GetActorLocation();

		if (!CachedAIController)
		{
			CachedAIController = Cast<APMCAIController>(GetController());
		}

		if (CachedAIController)
		{
			CachedAIController->SetLastKnownLocation(LastKnownPlayerLocation);
		}
	}

	if (CurrentState == EPMCState::Combat)
	{
		// Ya en combate: rotar hacia la dirección de impacto
		if (DamageInstigator)
		{
			FVector DirectionToAttacker = (DamageInstigator->GetActorLocation() - GetActorLocation()).GetSafeNormal();
			FRotator LookAtRotation = DirectionToAttacker.Rotation();
			SetActorRotation(FRotator(0.0f, LookAtRotation.Yaw, 0.0f));
		}
	}
	else
	{
		// No está en combate: transicionar a alerta y llamar refuerzos
		if (DamageInstigator)
		{
			if (!CachedAIController)
			{
				CachedAIController = Cast<APMCAIController>(GetController());
			}

			if (CachedAIController)
			{
				CachedAIController->SetTarget(DamageInstigator);
			}
		}

		SetState(EPMCState::Alert);

		// Llamar refuerzos al recibir daño inesperado
		if (StatConfig.bCallsForBackup)
		{
			CallForBackup();
		}
	}
}

// ------------------------------------------------------------
// REFUERZOS
// ------------------------------------------------------------

void APMCCharacter::CallForBackup()
{
	// Solo el servidor puede coordinar refuerzos
	if (!HasAuthority())
	{
		return;
	}

	// Sphere overlap para encontrar PMCs cercanos
	TArray<FOverlapResult> OverlapResults;
	FCollisionShape SphereShape = FCollisionShape::MakeSphere(StatConfig.MaxGroupCallRadius);
	FCollisionQueryParams QueryParams;
	QueryParams.AddIgnoredActor(this);

	GetWorld()->OverlapMultiByChannel(
		OverlapResults,
		GetActorLocation(),
		FQuat::Identity,
		ECollisionChannel::ECC_Pawn,
		SphereShape,
		QueryParams
	);

	// Activar alerta en todos los PMCs cercanos que no estén en combate o muertos
	for (const FOverlapResult& Result : OverlapResults)
	{
		APMCCharacter* NearbyPMC = Cast<APMCCharacter>(Result.GetActor());
		if (NearbyPMC && NearbyPMC != this)
		{
			EPMCState NearbyState = NearbyPMC->CurrentState;
			if (NearbyState != EPMCState::Combat && NearbyState != EPMCState::Dead)
			{
				// Pasar la última posición conocida antes de cambiar estado
				NearbyPMC->LastKnownPlayerLocation = LastKnownPlayerLocation;
				NearbyPMC->SetState(EPMCState::Alert);
			}
		}
	}
}

// ------------------------------------------------------------
// MUERTE
// ------------------------------------------------------------

void APMCCharacter::OnPMCDeath(AActor* Victim, AActor* Killer)
{
	// Solo el servidor procesa la muerte
	if (!HasAuthority())
	{
		return;
	}

	// Evitar procesamiento múltiple
	if (bIsDeathProcessed)
	{
		return;
	}
	bIsDeathProcessed = true;

	// Cambiar estado a Muerto
	CurrentState = EPMCState::Dead;
	UpdateAnimBP();

	// Desactivar colisión de la cápsula
	if (UCapsuleComponent* CapsuleComp = GetCapsuleComponent())
	{
		CapsuleComp->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	}

	// Activar simulación de física (ragdoll)
	if (USkeletalMeshComponent* MeshComp = GetMesh())
	{
		MeshComp->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
		MeshComp->SetCollisionProfileName(TEXT("Ragdoll"));
		MeshComp->SetSimulatePhysics(true);
	}

	// Detener movimiento
	if (UCharacterMovementComponent* MoveComp = GetCharacterMovement())
	{
		MoveComp->DisableMovement();
		MoveComp->StopMovementImmediately();
	}

	// Desactivar la IA: desposeer el peón del controlador
	if (!CachedAIController)
	{
		CachedAIController = Cast<APMCAIController>(GetController());
	}
	if (CachedAIController)
	{
		CachedAIController->UnPossess();
	}

	// TODO: Generar contenedor de botín en la posición de muerte
	// FActorSpawnParameters SpawnParams;
	// SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;
	// GetWorld()->SpawnActor<ALootContainer>(LootContainerClass, GetActorLocation(), FRotator::ZeroRotator, SpawnParams);

	UE_LOG(LogTemp, Log, TEXT("[PMC] %s ha muerto. Killer: %s"),
		*GetName(), Killer ? *Killer->GetName() : TEXT("Desconocido"));
}

// ------------------------------------------------------------
// CONSULTAS DE COMBATE
// ------------------------------------------------------------

FVector APMCCharacter::GetMuzzleLocation() const
{
	// Intentar obtener la posición del socket "Muzzle" del arma
	if (WeaponMesh && WeaponMesh->DoesSocketExist(FName("Muzzle")))
	{
		return WeaponMesh->GetSocketLocation(FName("Muzzle"));
	}

	// Fallback: usar la posición de la mano derecha del personaje
	if (GetMesh() && GetMesh()->DoesSocketExist(FName("hand_r_socket")))
	{
		return GetMesh()->GetSocketLocation(FName("hand_r_socket"));
	}

	// Último fallback: posición del actor más un offset hacia adelante
	return GetActorLocation() + GetActorForwardVector() * 100.0f + FVector(0.0f, 0.0f, 60.0f);
}

bool APMCCharacter::HasLineOfSightToPlayer() const
{
	// Obtener el jugador más cercano
	AZRPlayerCharacter* Player = GetNearestPlayer();
	if (!Player)
	{
		return false;
	}

	// Configurar el trace para ignorar al propio PMC y sus compañeros
	FCollisionQueryParams TraceParams(FName("PMC_LOS_Check"), true, this);
	TraceParams.bTraceComplex = false;

	// Ignorar a todos los PMCs para que no se bloqueen entre sí
	TArray<AActor*> AllPMCs;
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), APMCCharacter::StaticClass(), AllPMCs);
	for (AActor* PMCActor : AllPMCs)
	{
		TraceParams.AddIgnoredActor(PMCActor);
	}

	// Realizar trace desde la posición de los ojos del PMC al pecho del jugador
	FVector StartLocation = GetActorLocation() + FVector(0.0f, 0.0f, 60.0f); // Altura de ojos aproximada
	FVector EndLocation = Player->GetActorLocation() + FVector(0.0f, 0.0f, 50.0f); // Pecho del jugador

	FHitResult HitResult;
	bool bHit = GetWorld()->LineTraceSingleByChannel(
		HitResult,
		StartLocation,
		EndLocation,
		ECollisionChannel::ECC_Visibility,
		TraceParams
	);

	// Si no hay obstrucción, o el impacto fue en el propio jugador, hay línea de visión
	if (!bHit || HitResult.GetActor() == Player)
	{
		return true;
	}

	return false;
}

AZRPlayerCharacter* APMCCharacter::GetNearestPlayer() const
{
	// Buscar todos los jugadores en la escena
	TArray<AActor*> AllPlayers;
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), AZRPlayerCharacter::StaticClass(), AllPlayers);

	AZRPlayerCharacter* NearestPlayer = nullptr;
	float NearestDistanceSq = FLT_MAX;

	FVector MyLocation = GetActorLocation();

	for (AActor* PlayerActor : AllPlayers)
	{
		AZRPlayerCharacter* Player = Cast<AZRPlayerCharacter>(PlayerActor);
		if (Player && Player->IsAlive())
		{
			float DistSq = FVector::DistSquared(MyLocation, Player->GetActorLocation());
			if (DistSq < NearestDistanceSq)
			{
				NearestDistanceSq = DistSq;
				NearestPlayer = Player;
			}
		}
	}

	return NearestPlayer;
}

// ------------------------------------------------------------
// REPLICACION Y ANIMACION
// ------------------------------------------------------------

void APMCCharacter::OnRep_CurrentState()
{
	// Se ejecuta en clientes cuando el estado replicado cambia
	UpdateAnimBP();
}

void APMCCharacter::OnRep_LastKnownLocation()
{
	// Efectos visuales opcionales al actualizar la posición conocida en clientes
	// Por ejemplo: mostrar un marcador de posición en el minimapa
}

void APMCCharacter::UpdateAnimBP()
{
	// Notificar al Animation Blueprint del estado actual
	// El AnimBP leerá CurrentState para seleccionar la animación correcta
	// La implementación real se conecta mediante variables del AnimBP en el editor

	// TODO: Obtener el AnimInstance y actualizar variables de estado
	// if (UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance())
	// {
	//     if (UPMCAnimInstance* PMCAnimInstance = Cast<UPMCAnimInstance>(AnimInstance))
	//     {
	//         PMCAnimInstance->CurrentState = CurrentState;
	//     }
	// }
}
