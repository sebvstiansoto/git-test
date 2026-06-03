// ZRPlayerCharacter.cpp
// Implementación del personaje del jugador para ZonaRoja

#include "Characters/Player/ZRPlayerCharacter.h"
#include "Components/HealthComponent.h"
#include "Components/InventoryComponent.h"
#include "Components/StaminaComponent.h"
#include "Components/WeaponComponent.h"
#include "Components/InteractionComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "MotionWarpingComponent.h"
#include "Net/UnrealNetwork.h"

AZRPlayerCharacter::AZRPlayerCharacter()
{
	PrimaryActorTick.bCanEverTick = true;

	// ---------------------------------------------------------
	// CREAR Y CONFIGURAR COMPONENTES
	// ---------------------------------------------------------

	// Componente de salud por partes del cuerpo
	HealthComponent = CreateDefaultSubobject<UHealthComponent>(TEXT("HealthComponent"));
	HealthComponent->SetIsReplicated(true);

	// Inventario tipo cuadrícula
	InventoryComponent = CreateDefaultSubobject<UInventoryComponent>(TEXT("InventoryComponent"));
	InventoryComponent->SetIsReplicated(true);

	// Sistema de stamina para movimiento táctico
	StaminaComponent = CreateDefaultSubobject<UStaminaComponent>(TEXT("StaminaComponent"));
	StaminaComponent->SetIsReplicated(true);

	// Gestión de armas
	WeaponComponent = CreateDefaultSubobject<UWeaponComponent>(TEXT("WeaponComponent"));
	WeaponComponent->SetIsReplicated(true);

	// Detección de objetos interactuables
	InteractionComponent = CreateDefaultSubobject<UInteractionComponent>(TEXT("InteractionComponent"));

	// Brazo del resorte para la cámara
	SpringArmComponent = CreateDefaultSubobject<USpringArmComponent>(TEXT("SpringArmComponent"));
	SpringArmComponent->SetupAttachment(GetMesh(), FName(TEXT("head")));
	SpringArmComponent->TargetArmLength = 0.0f;          // Primera persona: sin longitud de brazo
	SpringArmComponent->bUsePawnControlRotation = true;   // Seguir la rotación del controlador
	SpringArmComponent->bDoCollisionTest = false;

	// Cámara principal del jugador
	CameraComponent = CreateDefaultSubobject<UCameraComponent>(TEXT("CameraComponent"));
	CameraComponent->SetupAttachment(SpringArmComponent, USpringArmComponent::SocketName);
	CameraComponent->bUsePawnControlRotation = false;     // El SpringArm ya rota

	// Motion Warping para animaciones contextuales (vault, mantle)
	MotionWarpingComponent = CreateDefaultSubobject<UMotionWarpingComponent>(TEXT("MotionWarpingComponent"));

	// Mesh de brazos en primera persona.
	// Se adjunta a la cámara para que siempre siga exactamente la vista del jugador.
	// Solo se muestra al dueño (SetOnlyOwnerSee=true en BeginPlay).
	FPPMesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("FPPMesh"));
	FPPMesh->SetupAttachment(CameraComponent);
	FPPMesh->SetRelativeLocation(FVector(0.0f, 0.0f, -160.0f)); // Offset para alinear con la cámara
	FPPMesh->bCastDynamicShadow = false;   // Los brazos FPP no proyectan sombra (evita artefactos)
	FPPMesh->CastShadow = false;

	// ---------------------------------------------------------
	// CONFIGURACION DE MOVIMIENTO
	// ---------------------------------------------------------

	WalkSpeed = 250.0f;     // Velocidad de patrulla táctico
	SprintSpeed = 520.0f;   // Sprint rápido (con equipo pesado reducida)
	AimingSpeed = 150.0f;   // Muy lento al apuntar
	CrouchSpeed = 160.0f;   // Lento agachado

	// Configurar el componente de movimiento del personaje
	if (UCharacterMovementComponent* CMC = GetCharacterMovement())
	{
		CMC->MaxWalkSpeed = WalkSpeed;
		CMC->MaxWalkSpeedCrouched = CrouchSpeed;
		CMC->JumpZVelocity = 420.0f;
		CMC->AirControl = 0.2f;
		CMC->GravityScale = 1.2f;         // Un poco más de gravedad para sensación táctica
		CMC->NavAgentProps.bCanCrouch = true;
		CMC->bOrientRotationToMovement = false; // El personaje rota manualmente
		CMC->bUseControllerDesiredRotation = false;
	}

	// El personaje usa el controlador para la rotación (no el vector de movimiento)
	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw = true;
	bUseControllerRotationRoll = false;

	// ---------------------------------------------------------
	// ESTADO INICIAL
	// ---------------------------------------------------------

	bIsSprinting = false;
	bIsAiming = false;
	LeanAmount = 0.0f;
	bIsProne = false;

	BaseTurnRate = 65.0f;
	BaseLookUpRate = 65.0f;
}

void AZRPlayerCharacter::BeginPlay()
{
	Super::BeginPlay();

	// Suscribirse al evento de muerte del HealthComponent
	if (HealthComponent)
	{
		HealthComponent->OnDeath.AddDynamic(this, &AZRPlayerCharacter::OnHealthDepleted);
	}

	// Activar el componente de interacción solo en el cliente local
	if (IsLocallyControlled() && InteractionComponent)
	{
		InteractionComponent->EnableInteraction();
	}

	// -------------------------------------------------------
	// CONFIGURACION DE VISIBILIDAD PRIMERA/TERCERA PERSONA
	// -------------------------------------------------------
	// El jugador LOCAL ve sus propios brazos (FPPMesh) pero NO su cuerpo completo.
	// Los demás jugadores ven el cuerpo completo (GetMesh()) pero NO los brazos FPP.
	// Esto evita que el jugador vea su propio cuerpo flotando frente a él.
	if (IsLocallyControlled())
	{
		// Brazos FPP: solo visibles para el dueño
		FPPMesh->SetOnlyOwnerSee(true);

		// Cuerpo TPP: invisible para el dueño (pero visible en shadow para los demás)
		GetMesh()->SetOwnerNoSee(true);
	}
	else
	{
		// Para otros jugadores: ocultar los brazos FPP (no son relevantes)
		FPPMesh->SetVisibility(false);
	}
}

void AZRPlayerCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	// Actualizar la velocidad de movimiento según el estado actual
	UpdateMovementSpeed();
}

void AZRPlayerCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);
	// La entrada se configura en RaidPlayerController
}

void AZRPlayerCharacter::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AZRPlayerCharacter, bIsSprinting);
	DOREPLIFETIME(AZRPlayerCharacter, bIsAiming);
	DOREPLIFETIME(AZRPlayerCharacter, LeanAmount);
	DOREPLIFETIME(AZRPlayerCharacter, bIsProne);
}

// ============================================================
// MOVIMIENTO
// ============================================================

void AZRPlayerCharacter::MoveForward(float Value)
{
	if (Controller && Value != 0.0f)
	{
		// Mover en la dirección que mira el controlador (ignorar pitch)
		const FRotator Rotation = Controller->GetControlRotation();
		const FRotator YawRotation(0, Rotation.Yaw, 0);
		const FVector Direction = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);
		AddMovementInput(Direction, Value);
	}
}

void AZRPlayerCharacter::MoveRight(float Value)
{
	if (Controller && Value != 0.0f)
	{
		const FRotator Rotation = Controller->GetControlRotation();
		const FRotator YawRotation(0, Rotation.Yaw, 0);
		const FVector Direction = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);
		AddMovementInput(Direction, Value);
	}
}

void AZRPlayerCharacter::LookRight(float Value)
{
	AddControllerYawInput(Value);
}

void AZRPlayerCharacter::LookUp(float Value)
{
	AddControllerPitchInput(-Value); // Invertir para comportamiento natural
}

void AZRPlayerCharacter::ToggleCrouch()
{
	if (bIsCrouched)
	{
		UnCrouch();
	}
	else
	{
		Crouch();

		// Cancelar sprint al agacharse
		if (bIsSprinting)
		{
			StopSprint();
		}
	}
}

void AZRPlayerCharacter::StartSprint()
{
	// No puede esprintar agachado, apuntando o sin stamina
	if (bIsCrouched || bIsAiming)
	{
		return;
	}

	if (StaminaComponent && StaminaComponent->StartSprinting())
	{
		bIsSprinting = true;

		if (!HasAuthority())
		{
			ServerSetSprinting(true);
		}

		// Cancelar el apuntado al esprintar
		if (bIsAiming)
		{
			StopAiming();
		}
	}
}

void AZRPlayerCharacter::StopSprint()
{
	if (bIsSprinting)
	{
		bIsSprinting = false;

		if (StaminaComponent)
		{
			StaminaComponent->StopSprinting();
		}

		if (!HasAuthority())
		{
			ServerSetSprinting(false);
		}
	}
}

void AZRPlayerCharacter::UpdateMovementSpeed()
{
	if (UCharacterMovementComponent* CMC = GetCharacterMovement())
	{
		float TargetSpeed = WalkSpeed;

		if (bIsSprinting)
		{
			TargetSpeed = SprintSpeed;
		}
		else if (bIsAiming)
		{
			TargetSpeed = AimingSpeed;
		}
		else if (bIsCrouched)
		{
			TargetSpeed = CrouchSpeed;
		}

		// Aplicar suavemente para evitar cambios bruscos de velocidad
		CMC->MaxWalkSpeed = FMath::FInterpTo(CMC->MaxWalkSpeed, TargetSpeed,
			GetWorld()->GetDeltaSeconds(), 10.0f);
	}
}

// ============================================================
// COMBATE
// ============================================================

void AZRPlayerCharacter::StartAiming()
{
	if (bIsSprinting)
	{
		StopSprint(); // El sprint cancela al apuntar
	}

	bIsAiming = true;

	if (WeaponComponent && WeaponComponent->GetCurrentWeapon())
	{
		// Notificar al arma que está en modo ADS
	}
}

void AZRPlayerCharacter::StopAiming()
{
	bIsAiming = false;
}

void AZRPlayerCharacter::StartFiring()
{
	if (bIsSprinting)
	{
		return; // No puede disparar esprintando
	}

	if (WeaponComponent)
	{
		WeaponComponent->StartFiring();
	}
}

void AZRPlayerCharacter::StopFiring()
{
	if (WeaponComponent)
	{
		WeaponComponent->StopFiring();
	}
}

void AZRPlayerCharacter::RequestReload()
{
	if (WeaponComponent)
	{
		WeaponComponent->StartReload();
	}
}

void AZRPlayerCharacter::SwitchToWeaponSlot(EEquipmentSlot Slot)
{
	if (WeaponComponent)
	{
		WeaponComponent->SwitchToWeaponSlot(Slot);
	}
}

void AZRPlayerCharacter::SetLeanAmount(float Value)
{
	// Limitar el valor de lean entre -1 y 1
	LeanAmount = FMath::Clamp(Value, -1.0f, 1.0f);

	if (!HasAuthority())
	{
		ServerSetLean(LeanAmount);
	}
}

void AZRPlayerCharacter::ToggleFireMode()
{
	if (WeaponComponent)
	{
		WeaponComponent->ToggleFireMode();
	}
}

// ============================================================
// CONSULTAS
// ============================================================

AActor* AZRPlayerCharacter::GetCurrentInteractionTarget() const
{
	if (InteractionComponent)
	{
		return InteractionComponent->GetCurrentInteractableActor();
	}
	return nullptr;
}

bool AZRPlayerCharacter::IsAlive() const
{
	if (HealthComponent)
	{
		return !HealthComponent->IsDead();
	}
	return true; // Asumir vivo si no hay componente de salud
}

// ============================================================
// DAÑO Y MUERTE
// ============================================================

float AZRPlayerCharacter::TakeDamage(float DamageAmount, FDamageEvent const& DamageEvent,
	AController* EventInstigator, AActor* DamageCauser)
{
	// El daño se procesa a través del HealthComponent
	// TakeDamage estándar de UE se deja para compatibilidad
	return Super::TakeDamage(DamageAmount, DamageEvent, EventInstigator, DamageCauser);
}

void AZRPlayerCharacter::HandleDeath(AActor* Killer)
{
	if (!HasAuthority())
	{
		return;
	}

	// Deshabilitar entrada del personaje
	if (AController* OwnerController = GetController())
	{
		DisableInput(Cast<APlayerController>(OwnerController));
	}

	// Activar ragdoll
	GetMesh()->SetSimulatePhysics(true);
	GetMesh()->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	// Deshabilitar movimiento
	if (UCharacterMovementComponent* CMC = GetCharacterMovement())
	{
		CMC->DisableMovement();
	}

	UE_LOG(LogTemp, Log, TEXT("[ZonaRoja] Personaje muerto: %s"), *GetName());
}

void AZRPlayerCharacter::OnHealthDepleted(AActor* Victim, AActor* Killer)
{
	HandleDeath(Killer);
}

void AZRPlayerCharacter::OnRep_PlayerState()
{
	Super::OnRep_PlayerState();
}

// ============================================================
// RPCS SERVIDOR
// ============================================================

void AZRPlayerCharacter::ServerSetLean_Implementation(float NewLeanAmount)
{
	LeanAmount = FMath::Clamp(NewLeanAmount, -1.0f, 1.0f);
}

void AZRPlayerCharacter::ServerSetSprinting_Implementation(bool bNewSprinting)
{
	bIsSprinting = bNewSprinting;
}
