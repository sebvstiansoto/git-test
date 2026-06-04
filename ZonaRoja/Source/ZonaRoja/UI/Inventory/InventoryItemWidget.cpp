// InventoryItemWidget.cpp
// Implementación del widget de item para la cuadrícula de inventario.

#include "UI/Inventory/InventoryItemWidget.h"
#include "UI/Inventory/InventoryDragDropOperation.h"
#include "Blueprint/WidgetBlueprintLibrary.h"
#include "Input/Reply.h"

FReply UInventoryItemWidget::NativeOnMouseButtonDown(
	const FGeometry& InGeometry,
	const FPointerEvent& InMouseEvent)
{
	FReply Reply = Super::NativeOnMouseButtonDown(InGeometry, InMouseEvent);

	if (InMouseEvent.GetEffectingButton() == EKeys::LeftMouseButton)
	{
		// Solicitar detección de arrastre al motor de UI
		bDetectingDrag = true;
		return Reply.DetectDrag(TakeWidget(), EKeys::LeftMouseButton);
	}

	return Reply;
}

FReply UInventoryItemWidget::NativeOnMouseButtonUp(
	const FGeometry& InGeometry,
	const FPointerEvent& InMouseEvent)
{
	FReply Reply = Super::NativeOnMouseButtonUp(InGeometry, InMouseEvent);

	if (InMouseEvent.GetEffectingButton() == EKeys::RightMouseButton)
	{
		// Emitir delegado para que el widget padre muestre el menú contextual
		OnRightClicked.Broadcast(this, ItemData);
		bDetectingDrag = false;
		return FReply::Handled();
	}

	bDetectingDrag = false;
	return Reply;
}

void UInventoryItemWidget::NativeOnDragDetected(
	const FGeometry& InGeometry,
	const FPointerEvent& InMouseEvent,
	UDragDropOperation*& OutOperation)
{
	Super::NativeOnDragDetected(InGeometry, InMouseEvent, OutOperation);

	// Crear la operación de arrastre con todos los datos del item
	UInventoryDragDropOperation* DragOp =
		NewObject<UInventoryDragDropOperation>(this, UInventoryDragDropOperation::StaticClass());

	if (DragOp)
	{
		DragOp->DraggedItem         = ItemData;
		DragOp->SourceRootSlot      = RootSlotIndex;
		DragOp->bFromEquipment      = false;
		DragOp->OriginalItemWidth   = ItemData.IsValid() ? 1 : 1; // El Blueprint establece el tamaño real
		DragOp->OriginalItemHeight  = 1;
		DragOp->bIsRotated          = bIsRotated;

		// Usar el propio widget como imagen visual durante el arrastre
		DragOp->DefaultDragVisual   = this;
		DragOp->Pivot               = EDragPivot::TopLeft;

		OutOperation = DragOp;
	}

	bDetectingDrag = false;
}

void UInventoryItemWidget::NativeOnMouseEnter(
	const FGeometry& InGeometry,
	const FPointerEvent& InMouseEvent)
{
	Super::NativeOnMouseEnter(InGeometry, InMouseEvent);
	OnHoverStart.Broadcast(this);
}

void UInventoryItemWidget::NativeOnMouseLeave(
	const FPointerEvent& InMouseEvent)
{
	Super::NativeOnMouseLeave(InMouseEvent);
	OnHoverEnd.Broadcast(this);
}
