// ZRHUDWidget.h
// Widget principal del HUD para ZonaRoja
// Muestra salud, munición, minimapa y estado de la incursión

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "ZRHUDWidget.generated.h"

/**
 * UZRHUDWidget
 * Widget base del HUD en pantalla durante la incursión.
 * La implementación visual se realiza en Blueprint.
 */
UCLASS()
class ZONAROJA_API UZRHUDWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	/**
	 * Actualiza el indicador de salud en el HUD.
	 * @param CurrentHealth Salud actual
	 * @param MaxHealth Salud máxima
	 */
	UFUNCTION(BlueprintImplementableEvent, Category = "HUD|Salud")
	void UpdateHealthDisplay(float CurrentHealth, float MaxHealth);

	/**
	 * Actualiza el contador de munición en el HUD.
	 * @param MagAmmo Munición en el cargador actual
	 * @param TotalAmmo Total de munición disponible
	 */
	UFUNCTION(BlueprintImplementableEvent, Category = "HUD|Munición")
	void UpdateAmmoDisplay(int32 MagAmmo, int32 TotalAmmo);

	/**
	 * Actualiza el temporizador de la incursión en el HUD.
	 * @param FormattedTime Tiempo en formato "MM:SS"
	 */
	UFUNCTION(BlueprintImplementableEvent, Category = "HUD|Tiempo")
	void UpdateRaidTimer(const FText& FormattedTime);

	/**
	 * Muestra una notificación temporal en pantalla.
	 * @param Message Texto del mensaje
	 */
	UFUNCTION(BlueprintImplementableEvent, Category = "HUD|Notificaciones")
	void ShowNotification(const FText& Message);

	/**
	 * Muestra u oculta la barra de progreso de extracción.
	 * @param bVisible Si debe mostrarse
	 * @param ExtractionTime Tiempo total de extracción (para la barra)
	 */
	UFUNCTION(BlueprintImplementableEvent, Category = "HUD|Extracción")
	void SetExtractionBarVisible(bool bVisible, float ExtractionTime = 5.0f);
};
