// RaidPlayerController.h
// Controlador del jugador para ZonaRoja
// Maneja la entrada con Enhanced Input, RPCs cliente-servidor y HUD

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "InputActionValue.h"
#include "RaidPlayerController.generated.h"

// Declaraciones anticipadas
class UInputMappingContext;
class UInputAction;
class UZRHUDWidget;
class UUserWidget;

/**
 * ARaidPlayerController
 * Controlador del jugador con soporte para Enhanced Input.
 * Gestiona la comunicación cliente-servidor para acciones de juego.
 */
UCLASS()
class ZONAROJA_API ARaidPlayerController : public APlayerController
{
	GENERATED_BODY()

public:
	ARaidPlayerController();

	// ---------------------------------------------------------
	// MAPAS DE ENTRADA (Enhanced Input)
	// ---------------------------------------------------------

	/** Contexto de entrada principal durante el juego */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Entrada|Contextos")
	TObjectPtr<UInputMappingContext> IMC_Default;

	/** Contexto de entrada para el menú de inventario */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Entrada|Contextos")
	TObjectPtr<UInputMappingContext> IMC_Inventory;

	/** Contexto de entrada para los menús del juego */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Entrada|Contextos")
	TObjectPtr<UInputMappingContext> IMC_Menu;

	// ---------------------------------------------------------
	// ACCIONES DE ENTRADA (Enhanced Input)
	// ---------------------------------------------------------

	/** Acción de movimiento (Vector2D: WASD) */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Entrada|Acciones")
	TObjectPtr<UInputAction> IA_Move;

	/** Acción de mirada (Vector2D: ratón/stick derecho) */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Entrada|Acciones")
	TObjectPtr<UInputAction> IA_Look;

	/** Acción de salto */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Entrada|Acciones")
	TObjectPtr<UInputAction> IA_Jump;

	/** Acción de agacharse */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Entrada|Acciones")
	TObjectPtr<UInputAction> IA_Crouch;

	/** Acción de tumbarse / posición prono */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Entrada|Acciones")
	TObjectPtr<UInputAction> IA_Prone;

	/** Acción de correr / sprint */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Entrada|Acciones")
	TObjectPtr<UInputAction> IA_Sprint;

	/** Acción de disparar (disparo primario) */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Entrada|Acciones")
	TObjectPtr<UInputAction> IA_Fire;

	/** Acción de apuntar (ADS - Aim Down Sights) */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Entrada|Acciones")
	TObjectPtr<UInputAction> IA_Aim;

	/** Acción de recargar el arma */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Entrada|Acciones")
	TObjectPtr<UInputAction> IA_Reload;

	/** Acción de cambiar al arma primaria */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Entrada|Acciones")
	TObjectPtr<UInputAction> IA_SwitchWeaponPrimary;

	/** Acción de cambiar al arma secundaria */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Entrada|Acciones")
	TObjectPtr<UInputAction> IA_SwitchWeaponSecondary;

	/** Acción de cambiar al arma de pistolera */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Entrada|Acciones")
	TObjectPtr<UInputAction> IA_SwitchWeaponHolster;

	/** Acción de interactuar con objetos del mundo */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Entrada|Acciones")
	TObjectPtr<UInputAction> IA_Interact;

	/** Acción de recoger objeto del suelo */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Entrada|Acciones")
	TObjectPtr<UInputAction> IA_PickupItem;

	/** Acción de abrir/cerrar el inventario */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Entrada|Acciones")
	TObjectPtr<UInputAction> IA_ToggleInventory;

	/** Acción de inclinarse a la izquierda (lean) */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Entrada|Acciones")
	TObjectPtr<UInputAction> IA_LeanLeft;

	/** Acción de inclinarse a la derecha (lean) */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Entrada|Acciones")
	TObjectPtr<UInputAction> IA_LeanRight;

	/** Acción de arrojar granada */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Entrada|Acciones")
	TObjectPtr<UInputAction> IA_ThrowGrenade;

	/** Acción de usar objeto médico rápido */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Entrada|Acciones")
	TObjectPtr<UInputAction> IA_UseHealItem;

	/** Acción de cambiar modo de fuego */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Entrada|Acciones")
	TObjectPtr<UInputAction> IA_ToggleFireMode;

	// ---------------------------------------------------------
	// RPCS SERVIDOR
	// ---------------------------------------------------------

	/**
	 * RPC para solicitar una interacción con un actor del mundo.
	 * @param InteractTarget Actor con el que se desea interactuar
	 */
	UFUNCTION(Server, Reliable, BlueprintCallable, Category = "RPC|Interacción")
	void ServerRequestInteract(AActor* InteractTarget);

	/**
	 * RPC para solicitar recoger un objeto del mundo.
	 * @param ItemActor Actor del objeto a recoger
	 */
	UFUNCTION(Server, Reliable, BlueprintCallable, Category = "RPC|Items")
	void ServerRequestPickupItem(AActor* ItemActor);

	/**
	 * RPC para solicitar el inicio de extracción desde una zona.
	 * @param ZoneID Identificador de la zona de extracción
	 */
	UFUNCTION(Server, Reliable, BlueprintCallable, Category = "RPC|Extracción")
	void ServerRequestStartExtraction(FName ZoneID);

	// ---------------------------------------------------------
	// RPCS CLIENTE (notificaciones del servidor al cliente)
	// ---------------------------------------------------------

	/**
	 * Muestra una notificación en la pantalla del cliente.
	 * @param NotificationText Texto a mostrar
	 */
	UFUNCTION(Client, Unreliable, BlueprintCallable, Category = "RPC|UI")
	void ClientShowNotification(const FText& NotificationText);

	/**
	 * Notifica al cliente que comenzó la extracción.
	 * @param ExtractionTime Tiempo total de extracción en segundos
	 */
	UFUNCTION(Client, Reliable, BlueprintCallable, Category = "RPC|Extracción")
	void ClientBeginExtractionCountdown(float ExtractionTime);

	/**
	 * Notifica al cliente que la extracción fue cancelada.
	 */
	UFUNCTION(Client, Reliable, BlueprintCallable, Category = "RPC|Extracción")
	void ClientCancelExtractionCountdown();

	// ---------------------------------------------------------
	// OVERRIDES
	// ---------------------------------------------------------
	virtual void BeginPlay() override;
	virtual void SetupInputComponent() override;
	virtual void OnPossess(APawn* InPawn) override;
	virtual void OnUnPossess() override;

protected:
	// ---------------------------------------------------------
	// HANDLERS DE ENTRADA
	// ---------------------------------------------------------

	/** Maneja la entrada de movimiento */
	void HandleMove(const FInputActionValue& Value);

	/** Maneja la entrada de mirada */
	void HandleLook(const FInputActionValue& Value);

	/** Maneja el inicio del salto */
	void HandleJumpStart();

	/** Maneja el final del salto */
	void HandleJumpStop();

	/** Maneja el toggle de agacharse */
	void HandleCrouchToggle();

	/** Maneja el inicio de sprint */
	void HandleSprintStart();

	/** Maneja el final de sprint */
	void HandleSprintStop();

	/** Maneja el inicio del disparo */
	void HandleFireStart();

	/** Maneja el final del disparo */
	void HandleFireStop();

	/** Maneja el inicio de apuntar */
	void HandleAimStart();

	/** Maneja el final de apuntar */
	void HandleAimStop();

	/** Maneja la recarga del arma */
	void HandleReload();

	/** Maneja el cambio al arma primaria */
	void HandleSwitchPrimary();

	/** Maneja el cambio al arma secundaria */
	void HandleSwitchSecondary();

	/** Maneja la interacción con el mundo */
	void HandleInteract();

	/** Maneja recoger objeto */
	void HandlePickupItem();

	/** Maneja abrir/cerrar inventario */
	void HandleToggleInventory();

	/** Maneja inclinarse a la izquierda */
	void HandleLeanLeft(const FInputActionValue& Value);

	/** Maneja inclinarse a la derecha */
	void HandleLeanRight(const FInputActionValue& Value);

	/** Maneja el cambio de modo de fuego */
	void HandleToggleFireMode();

	// ---------------------------------------------------------
	// WIDGET DEL HUD
	// ---------------------------------------------------------

	/** Referencia al widget principal del HUD */
	UPROPERTY(BlueprintReadOnly, Category = "UI")
	TObjectPtr<UZRHUDWidget> HUDWidget;

	/** Clase del widget del HUD a instanciar */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "UI")
	TSubclassOf<UZRHUDWidget> HUDWidgetClass;

	/** Crea e inicializa el HUD del jugador */
	void InitializeHUD();
};
