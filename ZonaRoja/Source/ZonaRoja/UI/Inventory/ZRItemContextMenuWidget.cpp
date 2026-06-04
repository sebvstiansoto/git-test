// ZRItemContextMenuWidget.cpp
// Implementación del menú contextual de items del inventario.

#include "UI/Inventory/ZRItemContextMenuWidget.h"
#include "Components/InventoryComponent.h"

// ============================================================
// CONTROL DE VISIBILIDAD
// ============================================================

void UZRItemContextMenuWidget::ShowForItem(
	const FItemData& Item,
	int32 RootSlot,
	UInventoryComponent* Inventory,
	FVector2D ScreenPosition)
{
	// Almacenar el contexto del item y el inventario
	ContextItem    = Item;
	ItemRootSlot   = RootSlot;
	OwningInventory = Inventory;

	// Determinar qué botones son relevantes para este item
	const bool bCanEquip   = CanItemBeEquipped();
	const bool bCanSplit   = (Item.StackCount > 1);

	// TODO: Consultar FItemDefinitionRow::bIsQuestItem del DataTable para
	// deshabilitar "Desechar" en items de misión. Por ahora siempre permitido.
	const bool bCanDiscard = true;

	// Hacer visible el widget antes de posicionarlo
	SetVisibility(ESlateVisibility::Visible);

	// Delegar visibilidad de botones y posición al Blueprint
	SetupMenuItems(bCanEquip, bCanSplit, bCanDiscard);
	PositionAt(ScreenPosition);
}

void UZRItemContextMenuWidget::Hide()
{
	SetVisibility(ESlateVisibility::Collapsed);

	// Limpiar el contexto al cerrar para evitar referencias colgantes
	ContextItem     = FItemData();
	ItemRootSlot    = -1;
	OwningInventory = nullptr;
}

// ============================================================
// ACCIONES DEL MENU
// ============================================================

void UZRItemContextMenuWidget::OnClickEquip()
{
	if (!OwningInventory || !ContextItem.IsValid())
	{
		Hide();
		return;
	}

	// TODO: Integración con DataTable pendiente — determinar la ranura óptima
	// consultando FItemDefinitionRow::ItemType para ContextItem.ItemDefinitionID.
	//
	// Lógica futura:
	//   FItemDefinitionRow* Row = DataTable->FindRow<FItemDefinitionRow>(
	//       ContextItem.ItemDefinitionID, ...);
	//   EEquipmentSlot BestSlot = DetermineSlotForType(Row->ItemType);
	//   OwningInventory->ServerRequestEquipItem(ContextItem.ID, BestSlot);
	//
	// Por ahora se intenta equipar en PrimaryWeapon como ranura por defecto.
	// El servidor rechazará la operación si no es compatible.
	OwningInventory->ServerRequestEquipItem(
		ContextItem.ID, EEquipmentSlot::PrimaryWeapon);

	Hide();
}

void UZRItemContextMenuWidget::OnClickDrop()
{
	if (!OwningInventory || !ContextItem.IsValid())
	{
		Hide();
		return;
	}

	// Solicitar al servidor que instancie un pickup en el mundo
	// y elimine el item del inventario
	OwningInventory->ServerRequestDropItem(ContextItem.ID);

	Hide();
}

void UZRItemContextMenuWidget::OnClickSplitStack()
{
	if (!OwningInventory || !ContextItem.IsValid() || ContextItem.StackCount <= 1)
	{
		Hide();
		return;
	}

	// TODO: Mostrar un slider o campo de texto para que el jugador elija la cantidad.
	// Por ahora se divide el stack por la mitad redondeando hacia abajo.
	// La mitad permanece en el slot original; el resto se coloca en el primer
	// slot libre disponible.
	const int32 SplitAmount = ContextItem.StackCount / 2;
	OwningInventory->ServerRequestSplitStack(ContextItem.ID, SplitAmount);

	Hide();
}

void UZRItemContextMenuWidget::OnClickInspect()
{
	if (!ContextItem.IsValid())
	{
		Hide();
		return;
	}

	// Notificar a los oyentes (p. ej. el sistema de inspección 3D)
	// que el jugador quiere inspeccionar este item
	OnInspectRequested.Broadcast(ContextItem);

	Hide();
}

void UZRItemContextMenuWidget::OnClickDiscard()
{
	if (!OwningInventory || !ContextItem.IsValid())
	{
		Hide();
		return;
	}

	// TODO: Mostrar popup de confirmación antes de descartar.
	// "¿Estás seguro de que quieres desechar [Nombre del Item]?  [Sí] [No]"
	// Por ahora se descarta directamente usando el RPC de tirar al suelo.
	// En producción, el servidor debería destruir el actor de pickup inmediatamente
	// en lugar de dejarlo accesible en el mundo.
	OwningInventory->ServerRequestDropItem(ContextItem.ID);

	Hide();
}

// ============================================================
// UTILIDADES INTERNAS
// ============================================================

bool UZRItemContextMenuWidget::CanItemBeEquipped() const
{
	// TODO: Integración con DataTable pendiente — consultar EItemType del item
	// para determinar si puede equiparse en alguna ranura de equipamiento.
	// Por ahora todos los items pueden intentar equiparse (el servidor rechaza
	// los incompatibles).
	return ContextItem.IsValid();
}
