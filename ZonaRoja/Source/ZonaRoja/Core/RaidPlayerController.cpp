// RaidPlayerController.cpp
// Implementación del controlador del jugador para ZonaRoja

#include "Core/RaidPlayerController.h"
#include "Characters/Player/ZRPlayerCharacter.h"
#include "UI/ZRHUDWidget.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "InputMappingContext.h"
#include "Blueprint/UserWidget.h"

ARaidPlayerController::ARaidPlayerController()
{
	// Los objetos UInputAction y UInputMappingContext se asignan en Blueprint
	// para mantener la flexibilidad del diseño de controles
}

void ARaidPlayerController::BeginPlay()
{
	Super::BeginPlay();

	// Configurar el contexto de entrada predeterminado solo en el cliente local
	if (IsLocalController())
	{
		if (UEnhancedInputLocalPlayerSubsystem* Subsystem =
			ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(GetLocalPlayer()))
		{
			if (IMC_Default)
			{
				// Prioridad 0: contexto base de juego
				Subsystem->AddMappingContext(IMC_Default, 0);
			}
		}

		// Crear el HUD solo en el cliente local
		InitializeHUD();
	}
}

void ARaidPlayerController::SetupInputComponent()
{
	Super::SetupInputComponent();

	// Obtener el componente de entrada mejorada
	if (UEnhancedInputComponent* EnhancedInput = Cast<UEnhancedInputComponent>(InputComponent))
	{
		// Movimiento y cámara
		if (IA_Move)
		{
			EnhancedInput->BindAction(IA_Move, ETriggerEvent::Triggered,
				this, &ARaidPlayerController::HandleMove);
		}
		if (IA_Look)
		{
			EnhancedInput->BindAction(IA_Look, ETriggerEvent::Triggered,
				this, &ARaidPlayerController::HandleLook);
		}

		// Salto
		if (IA_Jump)
		{
			EnhancedInput->BindAction(IA_Jump, ETriggerEvent::Started,
				this, &ARaidPlayerController::HandleJumpStart);
			EnhancedInput->BindAction(IA_Jump, ETriggerEvent::Completed,
				this, &ARaidPlayerController::HandleJumpStop);
		}

		// Agacharse
		if (IA_Crouch)
		{
			EnhancedInput->BindAction(IA_Crouch, ETriggerEvent::Started,
				this, &ARaidPlayerController::HandleCrouchToggle);
		}

		// Sprint
		if (IA_Sprint)
		{
			EnhancedInput->BindAction(IA_Sprint, ETriggerEvent::Started,
				this, &ARaidPlayerController::HandleSprintStart);
			EnhancedInput->BindAction(IA_Sprint, ETriggerEvent::Completed,
				this, &ARaidPlayerController::HandleSprintStop);
		}

		// Disparo
		if (IA_Fire)
		{
			EnhancedInput->BindAction(IA_Fire, ETriggerEvent::Started,
				this, &ARaidPlayerController::HandleFireStart);
			EnhancedInput->BindAction(IA_Fire, ETriggerEvent::Completed,
				this, &ARaidPlayerController::HandleFireStop);
		}

		// Apuntar
		if (IA_Aim)
		{
			EnhancedInput->BindAction(IA_Aim, ETriggerEvent::Started,
				this, &ARaidPlayerController::HandleAimStart);
			EnhancedInput->BindAction(IA_Aim, ETriggerEvent::Completed,
				this, &ARaidPlayerController::HandleAimStop);
		}

		// Recarga
		if (IA_Reload)
		{
			EnhancedInput->BindAction(IA_Reload, ETriggerEvent::Started,
				this, &ARaidPlayerController::HandleReload);
		}

		// Cambio de armas
		if (IA_SwitchWeaponPrimary)
		{
			EnhancedInput->BindAction(IA_SwitchWeaponPrimary, ETriggerEvent::Started,
				this, &ARaidPlayerController::HandleSwitchPrimary);
		}
		if (IA_SwitchWeaponSecondary)
		{
			EnhancedInput->BindAction(IA_SwitchWeaponSecondary, ETriggerEvent::Started,
				this, &ARaidPlayerController::HandleSwitchSecondary);
		}

		// Interacción y recogida
		if (IA_Interact)
		{
			EnhancedInput->BindAction(IA_Interact, ETriggerEvent::Started,
				this, &ARaidPlayerController::HandleInteract);
		}
		if (IA_PickupItem)
		{
			EnhancedInput->BindAction(IA_PickupItem, ETriggerEvent::Started,
				this, &ARaidPlayerController::HandlePickupItem);
		}

		// Inventario
		if (IA_ToggleInventory)
		{
			EnhancedInput->BindAction(IA_ToggleInventory, ETriggerEvent::Started,
				this, &ARaidPlayerController::HandleToggleInventory);
		}

		// Lean
		if (IA_LeanLeft)
		{
			EnhancedInput->BindAction(IA_LeanLeft, ETriggerEvent::Triggered,
				this, &ARaidPlayerController::HandleLeanLeft);
		}
		if (IA_LeanRight)
		{
			EnhancedInput->BindAction(IA_LeanRight, ETriggerEvent::Triggered,
				this, &ARaidPlayerController::HandleLeanRight);
		}

		// Modo de fuego
		if (IA_ToggleFireMode)
		{
			EnhancedInput->BindAction(IA_ToggleFireMode, ETriggerEvent::Started,
				this, &ARaidPlayerController::HandleToggleFireMode);
		}
	}
}

void ARaidPlayerController::OnPossess(APawn* InPawn)
{
	Super::OnPossess(InPawn);

	UE_LOG(LogTemp, Log, TEXT("[ZonaRoja] Controlador poseyendo peón: %s"), *InPawn->GetName());
}

void ARaidPlayerController::OnUnPossess()
{
	Super::OnUnPossess();
}

// ============================================================
// HANDLERS DE ENTRADA
// ============================================================

void ARaidPlayerController::HandleMove(const FInputActionValue& Value)
{
	if (AZRPlayerCharacter* PlayerChar = Cast<AZRPlayerCharacter>(GetPawn()))
	{
		const FVector2D MovementVector = Value.Get<FVector2D>();
		PlayerChar->MoveForward(MovementVector.Y);
		PlayerChar->MoveRight(MovementVector.X);
	}
}

void ARaidPlayerController::HandleLook(const FInputActionValue& Value)
{
	if (AZRPlayerCharacter* PlayerChar = Cast<AZRPlayerCharacter>(GetPawn()))
	{
		const FVector2D LookVector = Value.Get<FVector2D>();
		PlayerChar->LookRight(LookVector.X);
		PlayerChar->LookUp(LookVector.Y);
	}
}

void ARaidPlayerController::HandleJumpStart()
{
	if (ACharacter* Character = Cast<ACharacter>(GetPawn()))
	{
		Character->Jump();
	}
}

void ARaidPlayerController::HandleJumpStop()
{
	if (ACharacter* Character = Cast<ACharacter>(GetPawn()))
	{
		Character->StopJumping();
	}
}

void ARaidPlayerController::HandleCrouchToggle()
{
	if (AZRPlayerCharacter* PlayerChar = Cast<AZRPlayerCharacter>(GetPawn()))
	{
		PlayerChar->ToggleCrouch();
	}
}

void ARaidPlayerController::HandleSprintStart()
{
	if (AZRPlayerCharacter* PlayerChar = Cast<AZRPlayerCharacter>(GetPawn()))
	{
		PlayerChar->StartSprint();
	}
}

void ARaidPlayerController::HandleSprintStop()
{
	if (AZRPlayerCharacter* PlayerChar = Cast<AZRPlayerCharacter>(GetPawn()))
	{
		PlayerChar->StopSprint();
	}
}

void ARaidPlayerController::HandleFireStart()
{
	if (AZRPlayerCharacter* PlayerChar = Cast<AZRPlayerCharacter>(GetPawn()))
	{
		PlayerChar->StartFiring();
	}
}

void ARaidPlayerController::HandleFireStop()
{
	if (AZRPlayerCharacter* PlayerChar = Cast<AZRPlayerCharacter>(GetPawn()))
	{
		PlayerChar->StopFiring();
	}
}

void ARaidPlayerController::HandleAimStart()
{
	if (AZRPlayerCharacter* PlayerChar = Cast<AZRPlayerCharacter>(GetPawn()))
	{
		PlayerChar->StartAiming();
	}
}

void ARaidPlayerController::HandleAimStop()
{
	if (AZRPlayerCharacter* PlayerChar = Cast<AZRPlayerCharacter>(GetPawn()))
	{
		PlayerChar->StopAiming();
	}
}

void ARaidPlayerController::HandleReload()
{
	if (AZRPlayerCharacter* PlayerChar = Cast<AZRPlayerCharacter>(GetPawn()))
	{
		PlayerChar->RequestReload();
	}
}

void ARaidPlayerController::HandleSwitchPrimary()
{
	if (AZRPlayerCharacter* PlayerChar = Cast<AZRPlayerCharacter>(GetPawn()))
	{
		PlayerChar->SwitchToWeaponSlot(EEquipmentSlot::PrimaryWeapon);
	}
}

void ARaidPlayerController::HandleSwitchSecondary()
{
	if (AZRPlayerCharacter* PlayerChar = Cast<AZRPlayerCharacter>(GetPawn()))
	{
		PlayerChar->SwitchToWeaponSlot(EEquipmentSlot::SecondaryWeapon);
	}
}

void ARaidPlayerController::HandleInteract()
{
	if (AZRPlayerCharacter* PlayerChar = Cast<AZRPlayerCharacter>(GetPawn()))
	{
		// El componente de interacción gestiona el objetivo actual
		AActor* Target = PlayerChar->GetCurrentInteractionTarget();
		if (Target)
		{
			ServerRequestInteract(Target);
		}
	}
}

void ARaidPlayerController::HandlePickupItem()
{
	if (AZRPlayerCharacter* PlayerChar = Cast<AZRPlayerCharacter>(GetPawn()))
	{
		AActor* Target = PlayerChar->GetCurrentInteractionTarget();
		if (Target)
		{
			ServerRequestPickupItem(Target);
		}
	}
}

void ARaidPlayerController::HandleToggleInventory()
{
	// Alternar la visibilidad del widget de inventario
	if (IsLocalController() && HUDWidget)
	{
		// La lógica de mostrar/ocultar se implementa en el widget Blueprint
		// por ahora solo registramos la acción
		UE_LOG(LogTemp, Log, TEXT("[ZonaRoja] Toggle inventario"));
	}
}

void ARaidPlayerController::HandleLeanLeft(const FInputActionValue& Value)
{
	if (AZRPlayerCharacter* PlayerChar = Cast<AZRPlayerCharacter>(GetPawn()))
	{
		PlayerChar->SetLeanAmount(-Value.Get<float>());
	}
}

void ARaidPlayerController::HandleLeanRight(const FInputActionValue& Value)
{
	if (AZRPlayerCharacter* PlayerChar = Cast<AZRPlayerCharacter>(GetPawn()))
	{
		PlayerChar->SetLeanAmount(Value.Get<float>());
	}
}

void ARaidPlayerController::HandleToggleFireMode()
{
	if (AZRPlayerCharacter* PlayerChar = Cast<AZRPlayerCharacter>(GetPawn()))
	{
		PlayerChar->ToggleFireMode();
	}
}

// ============================================================
// RPCS SERVIDOR
// ============================================================

void ARaidPlayerController::ServerRequestInteract_Implementation(AActor* InteractTarget)
{
	if (!InteractTarget)
	{
		return;
	}

	// Validar que el jugador esté cerca del objeto de interacción
	if (AZRPlayerCharacter* PlayerChar = Cast<AZRPlayerCharacter>(GetPawn()))
	{
		const float Distance = FVector::Dist(
			PlayerChar->GetActorLocation(),
			InteractTarget->GetActorLocation());

		// Máximo 300cm de distancia para interactuar (3 metros)
		if (Distance <= 300.0f)
		{
			// Ejecutar la interfaz de interacción en el objeto objetivo
			// IZRInteractable::Execute_Interact(InteractTarget, PlayerChar);
			UE_LOG(LogTemp, Log, TEXT("[ZonaRoja][Servidor] Interacción con: %s"),
				*InteractTarget->GetName());
		}
	}
}

void ARaidPlayerController::ServerRequestPickupItem_Implementation(AActor* ItemActor)
{
	if (!ItemActor)
	{
		return;
	}

	UE_LOG(LogTemp, Log, TEXT("[ZonaRoja][Servidor] Recogiendo objeto: %s"),
		*ItemActor->GetName());
}

void ARaidPlayerController::ServerRequestStartExtraction_Implementation(FName ZoneID)
{
	// Notificar al GameMode para iniciar el proceso de extracción
	UE_LOG(LogTemp, Log, TEXT("[ZonaRoja][Servidor] Solicitud de extracción en zona: %s"),
		*ZoneID.ToString());
}

// ============================================================
// RPCS CLIENTE
// ============================================================

void ARaidPlayerController::ClientShowNotification_Implementation(const FText& NotificationText)
{
	// Mostrar la notificación en el HUD del cliente
	UE_LOG(LogTemp, Log, TEXT("[ZonaRoja][Cliente] Notificación: %s"),
		*NotificationText.ToString());
}

void ARaidPlayerController::ClientBeginExtractionCountdown_Implementation(float ExtractionTime)
{
	// Iniciar la UI de cuenta regresiva de extracción
	UE_LOG(LogTemp, Log, TEXT("[ZonaRoja][Cliente] Iniciando extracción: %.1f segundos"),
		ExtractionTime);
}

void ARaidPlayerController::ClientCancelExtractionCountdown_Implementation()
{
	// Cancelar y ocultar la UI de extracción
	UE_LOG(LogTemp, Log, TEXT("[ZonaRoja][Cliente] Extracción cancelada"));
}

// ============================================================
// INICIALIZACION DEL HUD
// ============================================================

void ARaidPlayerController::InitializeHUD()
{
	if (HUDWidgetClass && IsLocalController())
	{
		HUDWidget = Cast<UZRHUDWidget>(CreateWidget<UUserWidget>(this, HUDWidgetClass));
		if (HUDWidget)
		{
			HUDWidget->AddToViewport();
			UE_LOG(LogTemp, Log, TEXT("[ZonaRoja] HUD inicializado para el jugador local"));
		}
	}
}
