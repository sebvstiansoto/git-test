// InventoryTooltipWidget.h
// Tooltip flotante que muestra los detalles de un item al pasar el cursor.
// Muestra nombre, descripción, peso, valor, durabilidad y color de rareza.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Data/ZRTypes.h"
#include "InventoryTooltipWidget.generated.h"

/**
 * UInventoryTooltipWidget
 *
 * Widget flotante que aparece al situar el cursor sobre un item del inventario.
 * Muestra todos los metadatos relevantes del item con el color de rareza aplicado.
 *
 * Flujo de uso:
 *  1. UInventoryMainWidget llama a RefreshTooltip(Item) al recibir OnHoverStart.
 *  2. RefreshTooltip rellena todos los FText y calcula TierColor.
 *  3. RefreshTooltip llama a UpdateTooltipVisuals (Blueprint) para aplicar los valores
 *     a los elementos UMG (TextBlock, Image de borde de rareza, etc.).
 *  4. UInventoryMainWidget mueve el widget hasta la posición del cursor y lo hace visible.
 *  5. Al recibir OnHoverEnd, UInventoryMainWidget oculta el tooltip.
 *
 * Los campos de texto se exponen como BlueprintReadOnly para que el Blueprint
 * pueda enlazarlos directamente con sus TextBlocks.
 */
UCLASS(Abstract, BlueprintType, Blueprintable)
class ZONAROJA_API UInventoryTooltipWidget : public UUserWidget
{
	GENERATED_BODY()

public:

	// ============================================================
	// DATOS DEL ITEM (poblados por RefreshTooltip)
	// ============================================================

	/** Nombre visible del item — p. ej. "AK-103" */
	UPROPERTY(BlueprintReadOnly, Category = "Tooltip|Datos")
	FText ItemName;

	/** Descripción del item leída del DataTable */
	UPROPERTY(BlueprintReadOnly, Category = "Tooltip|Datos")
	FText ItemDescription;

	/** Peso formateado — p. ej. "3.8 kg" */
	UPROPERTY(BlueprintReadOnly, Category = "Tooltip|Datos")
	FText WeightText;

	/** Valor formateado — p. ej. "12.500 CZ" */
	UPROPERTY(BlueprintReadOnly, Category = "Tooltip|Datos")
	FText ValueText;

	/**
	 * Durabilidad formateada — p. ej. "87 / 100".
	 * Si la durabilidad es -1 (consumibles, llaves), se muestra "—".
	 */
	UPROPERTY(BlueprintReadOnly, Category = "Tooltip|Datos")
	FText DurabilityText;

	/** Nivel de rareza del item */
	UPROPERTY(BlueprintReadOnly, Category = "Tooltip|Datos")
	EItemTier ItemTier = EItemTier::Common;

	/** Color calculado según la rareza del item */
	UPROPERTY(BlueprintReadOnly, Category = "Tooltip|Datos")
	FLinearColor TierColor = FLinearColor::White;

	// ============================================================
	// METODOS PRINCIPALES
	// ============================================================

	/**
	 * Rellena todos los campos de texto y el color de rareza a partir de los datos
	 * del item. Llama a UpdateTooltipVisuals al terminar para que el Blueprint
	 * propague los valores a los elementos UMG.
	 *
	 * NOTA: El nombre, descripción, peso y valor se leen del DataTable
	 * DT_ItemDefinitions mediante ItemData.ItemDefinitionID.
	 * TODO: Integración con DataTable pendiente — actualmente usa valores de
	 *       marcador de posición cuando el DataTable no está disponible.
	 */
	UFUNCTION(BlueprintCallable, Category = "Tooltip")
	void RefreshTooltip(const FItemData& Item);

	/**
	 * Devuelve el color de borde/fondo asociado a un nivel de rareza.
	 *
	 * Mapa de colores:
	 *  Common    → Gris       (0.5, 0.5, 0.5)
	 *  Uncommon  → Verde      (0.1, 0.8, 0.1)
	 *  Rare      → Azul       (0.1, 0.4, 0.9)
	 *  Epic      → Púrpura    (0.6, 0.1, 0.9)
	 *  Legendary → Naranja    (1.0, 0.5, 0.0)
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Tooltip",
		meta = (DisplayName = "Obtener Color de Rareza"))
	static FLinearColor GetColorForTier(EItemTier Tier);

	// ============================================================
	// EVENTOS IMPLEMENTABLES EN BLUEPRINT
	// ============================================================

	/**
	 * Llamado por RefreshTooltip tras rellenar todos los campos.
	 * El Blueprint hijo lee ItemName, ItemDescription, WeightText, ValueText,
	 * DurabilityText y TierColor para actualizarlos en los elementos UMG.
	 */
	UFUNCTION(BlueprintImplementableEvent, Category = "Tooltip|Visual")
	void UpdateTooltipVisuals();
};
