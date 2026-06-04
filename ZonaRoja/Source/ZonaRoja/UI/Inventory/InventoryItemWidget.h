// InventoryItemWidget.h
// Widget de un item individual dentro de la cuadrícula del inventario.
// Su tamaño visual es (Ancho * TamañoCelda) x (Alto * TamañoCelda).

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Data/ZRTypes.h"
#include "InventoryItemWidget.generated.h"

class UInventoryDragDropOperation;

// ============================================================
// DELEGADOS
// ============================================================

/** Delegado emitido al hacer clic derecho sobre un item del inventario */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnItemRightClicked,
	UInventoryItemWidget*, Widget,
	const FItemData&, Item);

/** Delegado emitido cuando el cursor entra en el área del item */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnItemHoverStart,
	UInventoryItemWidget*, Widget);

/** Delegado emitido cuando el cursor sale del área del item */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnItemHoverEnd,
	UInventoryItemWidget*, Widget);

/**
 * UInventoryItemWidget
 *
 * Representa visualmente un item dentro de la cuadrícula del inventario.
 * Se redimensiona dinámicamente según (Ancho * TamañoCelda) x (Alto * TamañoCelda).
 * La parte visual (icono, borde de rareza, indicador de durabilidad, etc.)
 * se implementa en el Blueprint hijo mediante RefreshVisuals y SetHighlight.
 */
UCLASS(Abstract, BlueprintType, Blueprintable)
class ZONAROJA_API UInventoryItemWidget : public UUserWidget
{
	GENERATED_BODY()

public:

	// ============================================================
	// DATOS DEL ITEM
	// ============================================================

	/** Datos del item representado por este widget */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item")
	FItemData ItemData;

	/** Índice del slot raíz en la cuadrícula del inventario */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item")
	int32 RootSlotIndex = -1;

	/** Si el item está actualmente rotado en la cuadrícula */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item")
	bool bIsRotated = false;

	/** Tamaño de cada celda de la cuadrícula en píxeles */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item|Visual")
	float CellSize = 50.0f;

	// ============================================================
	// DELEGADOS PÚBLICOS
	// ============================================================

	/** Emitido al hacer clic derecho para abrir el menú contextual */
	UPROPERTY(BlueprintAssignable, Category = "Item|Delegados")
	FOnItemRightClicked OnRightClicked;

	/** Emitido cuando el cursor entra en el área del widget */
	UPROPERTY(BlueprintAssignable, Category = "Item|Delegados")
	FOnItemHoverStart OnHoverStart;

	/** Emitido cuando el cursor sale del área del widget */
	UPROPERTY(BlueprintAssignable, Category = "Item|Delegados")
	FOnItemHoverEnd OnHoverEnd;

	// ============================================================
	// EVENTOS IMPLEMENTABLES EN BLUEPRINT
	// ============================================================

	/**
	 * Actualiza todos los visuales del item (icono, color de rareza,
	 * indicador de durabilidad, cantidad de stack, etc.).
	 * Implementar en el Blueprint hijo.
	 */
	UFUNCTION(BlueprintImplementableEvent, BlueprintCallable, Category = "Item|Visual")
	void RefreshVisuals(const FItemData& Item, bool bRotated);

	/**
	 * Aplica o quita el resaltado de soltar sobre este item.
	 * @param bHighlight  Si se debe mostrar el resaltado.
	 * @param bIsValid    true = resaltado verde (posición válida),
	 *                    false = resaltado rojo (posición inválida).
	 */
	UFUNCTION(BlueprintImplementableEvent, BlueprintCallable, Category = "Item|Visual")
	void SetHighlight(bool bHighlight, bool bIsValid);

	// ============================================================
	// EVENTOS NATIVOS DE ENTRADA
	// ============================================================

	/**
	 * Clic izquierdo: inicia el arrastre del item creando
	 * una operación UInventoryDragDropOperation.
	 */
	virtual FReply NativeOnMouseButtonDown(
		const FGeometry& InGeometry,
		const FPointerEvent& InMouseEvent) override;

	/**
	 * Clic derecho: emite OnRightClicked para que el widget padre
	 * muestre el menú contextual.
	 */
	virtual FReply NativeOnMouseButtonUp(
		const FGeometry& InGeometry,
		const FPointerEvent& InMouseEvent) override;

	/** Detecta si se ha iniciado un arrastre con el botón presionado */
	virtual void NativeOnDragDetected(
		const FGeometry& InGeometry,
		const FPointerEvent& InMouseEvent,
		UDragDropOperation*& OutOperation) override;

	/** El cursor entra al área del widget — emite OnHoverStart */
	virtual void NativeOnMouseEnter(
		const FGeometry& InGeometry,
		const FPointerEvent& InMouseEvent) override;

	/** El cursor sale del área del widget — emite OnHoverEnd */
	virtual void NativeOnMouseLeave(
		const FPointerEvent& InMouseEvent) override;

protected:

	/** Indica si se debe detectar arrastre en el siguiente tick de ratón */
	bool bDetectingDrag = false;
};
