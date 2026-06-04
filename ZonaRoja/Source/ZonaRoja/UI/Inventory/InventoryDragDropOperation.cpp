// InventoryDragDropOperation.cpp
// Implementación de la operación de arrastre del inventario.

#include "UI/Inventory/InventoryDragDropOperation.h"

int32 UInventoryDragDropOperation::GetEffectiveWidth() const
{
	// Si está rotado, el alto original se convierte en el ancho efectivo
	return bIsRotated ? OriginalItemHeight : OriginalItemWidth;
}

int32 UInventoryDragDropOperation::GetEffectiveHeight() const
{
	// Si está rotado, el ancho original se convierte en el alto efectivo
	return bIsRotated ? OriginalItemWidth : OriginalItemHeight;
}

void UInventoryDragDropOperation::ToggleRotation()
{
	bIsRotated = !bIsRotated;
}
