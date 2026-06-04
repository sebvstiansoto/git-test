// EquipmentSlotWidget.h
// Ranura individual de equipamiento en el panel de silueta del personaje.
// Acepta items arrastrados desde la cuadrícula y permite desequipar con clic derecho.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Data/ZRTypes.h"
#include "EquipmentSlotWidget.generated.h"

class UInventoryComponent;
class UInventoryDragDropOperation;

// ============================================================
// DELEGADOS
// ============================================================

/**
 * Emitido al hacer clic derecho sobre una ranura de equipamiento ocupada.
 * El widget padre puede mostrar un menú contextual con opciones de desequipar.
 */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnEquipSlotRightClicked,
	UEquipmentSlotWidget*, Widget,
	EEquipmentSlot, Slot);

/**
 * UEquipmentSlotWidget
 *
 * Representa una ranura de equipamiento (casco, armadura, arma principal, etc.)
 * en el panel de silueta del personaje dentro de la pantalla de inventario.
 *
 * - Acepta arrastres de UInventoryDragDropOperation y llama a ServerRequestEquipItem.
 * - Resaltado verde/rojo durante el arrastre según la compatibilidad del item.
 * - Clic derecho en ranura ocupada llama a ServerRequestUnequipItem.
 * - Los visuales (icono, borde, estado vacío) se implementan en el Blueprint hijo.
 */
UCLASS(Abstract, BlueprintType, Blueprintable)
class ZONAROJA_API UEquipmentSlotWidget : public UUserWidget
{
	GENERATED_BODY()

public:

	// ============================================================
	// CONFIGURACION DE LA RANURA
	// ============================================================

	/** Tipo de ranura de equipamiento que representa este widget */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Equipamiento")
	EEquipmentSlot SlotType = EEquipmentSlot::PrimaryWeapon;

	// ============================================================
	// ESTADO
	// ============================================================

	/** Datos del item actualmente equipado en esta ranura. ID inválido = ranura vacía */
	UPROPERTY(BlueprintReadWrite, Category = "Equipamiento|Estado")
	FItemData CurrentItem;

	/** Referencia al componente de inventario del jugador propietario */
	UPROPERTY(BlueprintReadWrite, Category = "Equipamiento|Referencias")
	TObjectPtr<UInventoryComponent> OwningInventory;

	// ============================================================
	// DELEGADOS PÚBLICOS
	// ============================================================

	/** Emitido al hacer clic derecho sobre la ranura (para menú contextual) */
	UPROPERTY(BlueprintAssignable, Category = "Equipamiento|Delegados")
	FOnEquipSlotRightClicked OnEquipSlotRightClicked;

	// ============================================================
	// EVENTOS IMPLEMENTABLES EN BLUEPRINT
	// ============================================================

	/**
	 * Actualiza los visuales de la ranura con el item equipado.
	 * Si Item.IsValid() es false, muestra la ranura vacía.
	 * Implementar en el Blueprint hijo.
	 */
	UFUNCTION(BlueprintImplementableEvent, BlueprintCallable, Category = "Equipamiento|Visual")
	void RefreshSlotVisuals(const FItemData& Item, EEquipmentSlot InSlot);

	/**
	 * Aplica o quita el resaltado de arrastre sobre la ranura.
	 * @param bHighlight  Si debe mostrarse el resaltado.
	 * @param bIsValid    true = verde (item compatible), false = rojo (incompatible).
	 */
	UFUNCTION(BlueprintImplementableEvent, BlueprintCallable, Category = "Equipamiento|Visual")
	void SetDragHoverHighlight(bool bHighlight, bool bIsValid);

	// ============================================================
	// EVENTOS NATIVOS DE DRAG & DROP E ENTRADA
	// ============================================================

	/**
	 * El cursor entra en la ranura con un item arrastrado.
	 * Muestra el resaltado según la compatibilidad del tipo de item.
	 */
	virtual bool NativeOnDragOver(
		const FGeometry& InGeometry,
		const FDragDropEvent& InDragDropEvent,
		UDragDropOperation* InOperation) override;

	/**
	 * El jugador suelta el item sobre esta ranura.
	 * Si el tipo es compatible, llama a ServerRequestEquipItem.
	 */
	virtual bool NativeOnDrop(
		const FGeometry& InGeometry,
		const FDragDropEvent& InDragDropEvent,
		UDragDropOperation* InOperation) override;

	/**
	 * El cursor sale de la ranura durante un arrastre.
	 * Quita el resaltado.
	 */
	virtual void NativeOnDragLeave(
		const FDragDropEvent& InDragDropEvent,
		UDragDropOperation* InOperation) override;

	/**
	 * Clic derecho sobre una ranura ocupada: llama a ServerRequestUnequipItem
	 * y emite OnEquipSlotRightClicked para el menú contextual.
	 */
	virtual FReply NativeOnMouseButtonDown(
		const FGeometry& InGeometry,
		const FPointerEvent& InMouseEvent) override;

protected:

	/**
	 * Determina si un tipo de item es compatible con esta ranura de equipamiento.
	 * Se utiliza para decidir si mostrar resaltado verde o rojo durante el arrastre.
	 */
	bool IsItemCompatibleWithSlot(const FItemData& Item) const;
};
