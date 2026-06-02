// EquipmentSlotWidget.cpp
// Implementación de la ranura de equipamiento en el panel de silueta.

#include "UI/Inventory/EquipmentSlotWidget.h"
#include "UI/Inventory/InventoryDragDropOperation.h"
#include "Components/InventoryComponent.h"

// ============================================================
// DRAG & DROP
// ============================================================

bool UEquipmentSlotWidget::NativeOnDragOver(
	const FGeometry& InGeometry,
	const FDragDropEvent& InDragDropEvent,
	UDragDropOperation* InOperation)
{
	Super::NativeOnDragOver(InGeometry, InDragDropEvent, InOperation);

	const UInventoryDragDropOperation* DragOp =
		Cast<UInventoryDragDropOperation>(InOperation);

	if (!DragOp)
	{
		return false;
	}

	// Mostrar resaltado con el color apropiado según la compatibilidad del item
	const bool bCompatible = IsItemCompatibleWithSlot(DragOp->DraggedItem);
	SetDragHoverHighlight(true, bCompatible);

	return true;
}

bool UEquipmentSlotWidget::NativeOnDrop(
	const FGeometry& InGeometry,
	const FDragDropEvent& InDragDropEvent,
	UDragDropOperation* InOperation)
{
	Super::NativeOnDrop(InGeometry, InDragDropEvent, InOperation);

	const UInventoryDragDropOperation* DragOp =
		Cast<UInventoryDragDropOperation>(InOperation);

	if (!DragOp || !OwningInventory)
	{
		SetDragHoverHighlight(false, false);
		return false;
	}

	// Solo equipar si el tipo de item es compatible con esta ranura
	if (!IsItemCompatibleWithSlot(DragOp->DraggedItem))
	{
		SetDragHoverHighlight(false, false);
		return false;
	}

	// Solicitar al servidor que equipe el item en esta ranura
	OwningInventory->ServerRequestEquipItem(DragOp->DraggedItem.ID, SlotType);

	SetDragHoverHighlight(false, false);
	return true;
}

void UEquipmentSlotWidget::NativeOnDragLeave(
	const FDragDropEvent& InDragDropEvent,
	UDragDropOperation* InOperation)
{
	Super::NativeOnDragLeave(InDragDropEvent, InOperation);

	// Quitar el resaltado al salir de la ranura
	SetDragHoverHighlight(false, false);
}

// ============================================================
// ENTRADA DE RATON
// ============================================================

FReply UEquipmentSlotWidget::NativeOnMouseButtonDown(
	const FGeometry& InGeometry,
	const FPointerEvent& InMouseEvent)
{
	FReply Reply = Super::NativeOnMouseButtonDown(InGeometry, InMouseEvent);

	if (InMouseEvent.GetEffectingButton() == EKeys::RightMouseButton)
	{
		// Solo actuar si la ranura tiene un item equipado
		if (CurrentItem.IsValid() && OwningInventory)
		{
			// Emitir delegado para que el padre muestre el menú contextual
			OnEquipSlotRightClicked.Broadcast(this, SlotType);

			// Solicitar al servidor que desequipe y devuelva el item al inventario
			OwningInventory->ServerRequestUnequipItem(SlotType);
			return FReply::Handled();
		}
	}

	return Reply;
}

// ============================================================
// UTILIDADES INTERNAS
// ============================================================

bool UEquipmentSlotWidget::IsItemCompatibleWithSlot(const FItemData& Item) const
{
	// TODO: Integración con DataTable pendiente — esta lógica debe consultar
	// FItemDefinitionRow::ItemType para cada ItemDefinitionID y compararlo
	// con la ranura. Por ahora se aplican reglas básicas por tipo de ranura.

	// Mapa aproximado de ranura → tipos de item aceptados
	// (refinar cuando DataTable DT_ItemDefinitions esté disponible)
	switch (SlotType)
	{
	case EEquipmentSlot::PrimaryWeapon:
	case EEquipmentSlot::SecondaryWeapon:
		// Las ranuras de arma aceptan armas
		return true; // Sin DataTable no podemos filtrar por EItemType::Weapon

	case EEquipmentSlot::Holster:
		return true;

	case EEquipmentSlot::Helmet:
	case EEquipmentSlot::BodyArmor:
	case EEquipmentSlot::Vest:
	case EEquipmentSlot::Backpack:
	case EEquipmentSlot::LeftPocket:
	case EEquipmentSlot::RightPocket:
		return true;

	default:
		return false;
	}
}
