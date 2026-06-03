// InventoryMainWidget.cpp
// Implementación del widget raíz de la pantalla de inventario.
// Coordina todos los sub-widgets y gestiona los eventos del sistema de inventario.

#include "UI/Inventory/InventoryMainWidget.h"
#include "UI/Inventory/InventoryGridWidget.h"
#include "UI/Inventory/InventoryTooltipWidget.h"
#include "UI/Inventory/ZRItemContextMenuWidget.h"
#include "UI/Inventory/EquipmentSlotWidget.h"
#include "UI/Inventory/InventoryItemWidget.h"
#include "UI/Inventory/InventoryDragDropOperation.h"
#include "Components/InventoryComponent.h"
#include "Blueprint/WidgetBlueprintLibrary.h"
#include "Framework/Application/SlateApplication.h"

// ============================================================
// CICLO DE VIDA DEL WIDGET
// ============================================================

void UInventoryMainWidget::NativeConstruct()
{
	Super::NativeConstruct();

	// Ocultar el tooltip y el menú contextual al abrir el inventario
	if (TooltipWidget)
	{
		TooltipWidget->SetVisibility(ESlateVisibility::Collapsed);
	}

	if (ContextMenuWidget)
	{
		ContextMenuWidget->SetVisibility(ESlateVisibility::Collapsed);
	}

	// Notificar al Blueprint para reproducir la animación de apertura
	OnInventoryOpened();
}

void UInventoryMainWidget::NativeDestruct()
{
	// Desvincular todos los delegados del componente de inventario
	if (OwningInventory)
	{
		OwningInventory->OnInventoryChanged.RemoveDynamic(
			this, &UInventoryMainWidget::HandleInventoryChanged);

		OwningInventory->OnWeightChanged.RemoveDynamic(
			this, &UInventoryMainWidget::HandleWeightChanged);

		OwningInventory->OnItemAdded.RemoveDynamic(
			this, &UInventoryMainWidget::HandleItemAdded);

		OwningInventory->OnEquipmentChanged.RemoveDynamic(
			this, &UInventoryMainWidget::HandleEquipmentChanged);
	}

	// Reproducir animación de cierre antes de destruir (si el Blueprint lo implementa)
	OnInventoryClosed();

	Super::NativeDestruct();
}

// ============================================================
// INICIALIZACION
// ============================================================

void UInventoryMainWidget::InitializeInventory(UInventoryComponent* Inventory)
{
	if (!Inventory)
	{
		UE_LOG(LogTemp, Warning,
			TEXT("UInventoryMainWidget::InitializeInventory — Inventory es nulo"));
		return;
	}

	OwningInventory = Inventory;

	// ---------------------------------------------------
	// Inicializar la cuadrícula principal
	// ---------------------------------------------------
	if (GridWidget)
	{
		GridWidget->OwningInventory = Inventory;
		GridWidget->InitializeGrid(Inventory);
	}

	// ---------------------------------------------------
	// Propagar la referencia de inventario al menú contextual
	// ---------------------------------------------------
	if (ContextMenuWidget)
	{
		ContextMenuWidget->OwningInventory = Inventory;
	}

	// ---------------------------------------------------
	// Inicializar los widgets de ranura de equipamiento
	// ---------------------------------------------------
	for (UEquipmentSlotWidget* SlotWidget : EquipmentSlotWidgets)
	{
		if (SlotWidget)
		{
			SlotWidget->OwningInventory = Inventory;

			// El desequipado se gestiona directamente en NativeOnMouseButtonDown
			// de UEquipmentSlotWidget mediante ServerRequestUnequipItem.
			// El delegado OnEquipSlotRightClicked está disponible para que
			// el Blueprint del widget padre lo vincule si necesita opciones extras.
		}
	}

	// Refrescar visualmente todas las ranuras de equipo con el estado actual
	RefreshAllEquipmentSlots();

	// ---------------------------------------------------
	// Vincular delegados del InventoryComponent
	// ---------------------------------------------------
	Inventory->OnInventoryChanged.AddDynamic(
		this, &UInventoryMainWidget::HandleInventoryChanged);

	Inventory->OnWeightChanged.AddDynamic(
		this, &UInventoryMainWidget::HandleWeightChanged);

	Inventory->OnItemAdded.AddDynamic(
		this, &UInventoryMainWidget::HandleItemAdded);

	Inventory->OnEquipmentChanged.AddDynamic(
		this, &UInventoryMainWidget::HandleEquipmentChanged);

	// ---------------------------------------------------
	// Vincular los delegados de los widgets de item del grid
	// Lo hacemos tras InitializeGrid para que ActiveItemWidgets esté poblado
	// ---------------------------------------------------
	if (GridWidget)
	{
		for (UInventoryItemWidget* ItemWidget : GridWidget->ActiveItemWidgets)
		{
			BindItemWidgetEvents(ItemWidget);
		}
	}

	// Estado inicial de peso y espacio
	RefreshWeightDisplay();
	RefreshFreeSpace();
}

// ============================================================
// REFRESCO DE INFORMACION
// ============================================================

void UInventoryMainWidget::RefreshWeightDisplay()
{
	if (!OwningInventory)
	{
		WeightDisplayText = FText::FromString(TEXT("? / ? kg"));
		return;
	}

	const float Current = OwningInventory->GetCurrentWeightKg();
	const float Max     = OwningInventory->GetMaxWeightKg();

	// Formato: "32.4 / 40.0 kg"
	WeightDisplayText = FText::Format(
		NSLOCTEXT("ZonaRoja", "PesoDisplay", "{0} / {1} kg"),
		FText::AsNumber(FMath::RoundToFloat(Current * 10.0f) / 10.0f),
		FText::AsNumber(FMath::RoundToFloat(Max     * 10.0f) / 10.0f));
}

void UInventoryMainWidget::RefreshFreeSpace()
{
	if (!OwningInventory)
	{
		FreeSpaceText = NSLOCTEXT("ZonaRoja", "EspacioND", "? celdas libres");
		return;
	}

	const int32 FreeSlots = OwningInventory->GetFreeSlotCount();

	// Formato: "47 celdas libres"
	FreeSpaceText = FText::Format(
		NSLOCTEXT("ZonaRoja", "CeldasLibres", "{0} celdas libres"),
		FText::AsNumber(FreeSlots));
}

// ============================================================
// ENTRADA DE TECLADO
// ============================================================

FReply UInventoryMainWidget::NativeOnKeyDown(
	const FGeometry& InGeometry,
	const FKeyEvent& InKeyEvent)
{
	// R durante un arrastre activo rota el item
	if (InKeyEvent.GetKey() == EKeys::R)
	{
		// Buscar la operación de arrastre activa en el framework de Slate
		TSharedPtr<FDragDropOperation> ActiveDragOp =
			FSlateApplication::Get().GetDragDroppingContent();

		if (ActiveDragOp.IsValid())
		{
			// El sistema de drag de Slate usa un tipo propio;
			// UE5 expone UDragDropOperation a través de UWidgetBlueprintLibrary.
			// Necesitamos recuperar la operación UObject desde el contexto actual.
			// La forma correcta es usar el puntero que UUserWidget trackea internamente.
			// Accedemos a él a través del sistema de arrastre de UMG.
			UDragDropOperation* UMGDragOp =
				UWidgetBlueprintLibrary::GetDragDroppingContent();

			UInventoryDragDropOperation* InvDragOp =
				Cast<UInventoryDragDropOperation>(UMGDragOp);

			if (InvDragOp)
			{
				InvDragOp->ToggleRotation();
				return FReply::Handled();
			}
		}
	}

	return Super::NativeOnKeyDown(InGeometry, InKeyEvent);
}

// ============================================================
// CALLBACKS DE DELEGADOS
// ============================================================

void UInventoryMainWidget::HandleInventoryChanged()
{
	// El inventario cambió — refrescar contadores de peso y espacio libre
	RefreshWeightDisplay();
	RefreshFreeSpace();

	// Refrescar ranuras de equipamiento por si hubo cambios de equipo
	RefreshAllEquipmentSlots();

	// La cuadrícula se reconstruye por su propio binding a OnInventoryChanged
	// (vinculado en InitializeGrid). No necesitamos llamarlo de nuevo aquí.

	// Revincular los delegados de los widgets de item del grid
	// ya que RebuildItemWidgets recrea todos los widgets
	if (GridWidget)
	{
		for (UInventoryItemWidget* ItemWidget : GridWidget->ActiveItemWidgets)
		{
			BindItemWidgetEvents(ItemWidget);
		}
	}
}

void UInventoryMainWidget::HandleWeightChanged(float NewWeightKg)
{
	if (!OwningInventory)
	{
		return;
	}

	const float Max = OwningInventory->GetMaxWeightKg();

	// Formato: "32.4 / 40.0 kg"
	WeightDisplayText = FText::Format(
		NSLOCTEXT("ZonaRoja", "PesoDisplay", "{0} / {1} kg"),
		FText::AsNumber(FMath::RoundToFloat(NewWeightKg * 10.0f) / 10.0f),
		FText::AsNumber(FMath::RoundToFloat(Max          * 10.0f) / 10.0f));
}

void UInventoryMainWidget::HandleItemAdded(
	const FItemData& AddedItem, int32 RootSlotIndex)
{
	// Reproducir animación de aparición en el slot recién ocupado
	PlayItemAddedAnimation(RootSlotIndex);

	// Refrescar el contador de espacio libre
	RefreshFreeSpace();
}

void UInventoryMainWidget::HandleItemRightClicked(
	UInventoryItemWidget* Widget, const FItemData& Item)
{
	if (!ContextMenuWidget)
	{
		return;
	}

	// Calcular el índice raíz del item (puede provenir del widget)
	const int32 RootSlot = Widget ? Widget->RootSlotIndex : -1;

	// Mostrar el menú contextual en la posición actual del cursor
	ContextMenuWidget->ShowForItem(
		Item,
		RootSlot,
		OwningInventory,
		GetMouseScreenPosition());
}

void UInventoryMainWidget::HandleItemHoverStart(UInventoryItemWidget* Widget)
{
	if (!TooltipWidget || !Widget)
	{
		return;
	}

	// Rellenar el tooltip con los datos del item
	TooltipWidget->RefreshTooltip(Widget->ItemData);

	// Posicionar el tooltip cerca del cursor (con un offset para no tapar el item)
	const FVector2D MousePos = GetMouseScreenPosition();
	const FVector2D TooltipOffset(15.0f, 10.0f); // Desplazamiento del cursor

	// Hacer visible el tooltip
	TooltipWidget->SetVisibility(ESlateVisibility::HitTestInvisible);

	// La posición exacta del tooltip se delega al Blueprint que puede
	// ajustar los márgenes para que no salga de los límites de la pantalla.
	// Aquí establecemos el renderizado de posición del widget.
	TooltipWidget->SetRenderTranslation(MousePos + TooltipOffset);
}

void UInventoryMainWidget::HandleItemHoverEnd(UInventoryItemWidget* Widget)
{
	if (TooltipWidget)
	{
		TooltipWidget->SetVisibility(ESlateVisibility::Collapsed);
	}
}

void UInventoryMainWidget::HandleEquipmentChanged(
	EEquipmentSlot ChangedSlot, const FItemData& NewItem)
{
	// Encontrar el widget de ranura correspondiente y actualizarlo
	for (UEquipmentSlotWidget* SlotWidget : EquipmentSlotWidgets)
	{
		if (SlotWidget && SlotWidget->SlotType == ChangedSlot)
		{
			SlotWidget->CurrentItem = NewItem;
			SlotWidget->RefreshSlotVisuals(NewItem, ChangedSlot);
			break;
		}
	}

	// Actualizar también el peso y el espacio por si el equipo modificó la cuadrícula
	RefreshWeightDisplay();
	RefreshFreeSpace();
}

// ============================================================
// UTILIDADES INTERNAS
// ============================================================

void UInventoryMainWidget::BindItemWidgetEvents(UInventoryItemWidget* ItemWidget)
{
	if (!ItemWidget)
	{
		return;
	}

	// Evitar vincular el mismo widget más de una vez
	// (AddDynamic es seguro de llamar múltiples veces en UE5 — ignora duplicados)
	ItemWidget->OnRightClicked.AddDynamic(
		this, &UInventoryMainWidget::HandleItemRightClicked);

	ItemWidget->OnHoverStart.AddDynamic(
		this, &UInventoryMainWidget::HandleItemHoverStart);

	ItemWidget->OnHoverEnd.AddDynamic(
		this, &UInventoryMainWidget::HandleItemHoverEnd);
}

void UInventoryMainWidget::RefreshAllEquipmentSlots()
{
	if (!OwningInventory)
	{
		return;
	}

	for (UEquipmentSlotWidget* SlotWidget : EquipmentSlotWidgets)
	{
		if (!SlotWidget)
		{
			continue;
		}

		// Obtener el item equipado en esta ranura desde el componente
		FItemData EquippedItem;
		const bool bHasItem =
			OwningInventory->GetEquippedItem(SlotWidget->SlotType, EquippedItem);

		SlotWidget->CurrentItem = bHasItem ? EquippedItem : FItemData();

		// Pasar el item (válido o inválido) al Blueprint para actualizar visuales
		// Si EquippedItem.IsValid() es false, el Blueprint mostrará el estado vacío
		SlotWidget->RefreshSlotVisuals(SlotWidget->CurrentItem, SlotWidget->SlotType);
	}
}

FVector2D UInventoryMainWidget::GetMouseScreenPosition() const
{
	// Obtener la posición del cursor en coordenadas de pantalla a través de Slate
	const FVector2D MousePos =
		FSlateApplication::Get().GetCursorPos();

	return MousePos;
}
