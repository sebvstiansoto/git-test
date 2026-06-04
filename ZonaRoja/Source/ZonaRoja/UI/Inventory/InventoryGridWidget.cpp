// InventoryGridWidget.cpp
// Implementación de la cuadrícula principal del inventario.
// Gestiona la creación de widgets de item, el posicionado en el canvas,
// y las operaciones de arrastrar y soltar contra el servidor.

#include "UI/Inventory/InventoryGridWidget.h"
#include "UI/Inventory/InventoryDragDropOperation.h"
#include "UI/Inventory/InventoryItemWidget.h"
#include "Components/InventoryComponent.h"
#include "Components/CanvasPanel.h"
#include "Components/CanvasPanelSlot.h"
#include "Blueprint/WidgetLayoutLibrary.h"
#include "Blueprint/WidgetBlueprintLibrary.h"

// ============================================================
// INICIALIZACION
// ============================================================

void UInventoryGridWidget::InitializeGrid(UInventoryComponent* Inventory)
{
	if (!Inventory)
	{
		UE_LOG(LogTemp, Warning, TEXT("UInventoryGridWidget::InitializeGrid — Inventory es nulo"));
		return;
	}

	OwningInventory = Inventory;

	// Vincular al delegado de cambio general para reconstruir la cuadrícula
	// cuando el servidor replique cambios al cliente
	OwningInventory->OnInventoryChanged.AddDynamic(
		this, &UInventoryGridWidget::HandleInventoryChanged);

	RebuildItemWidgets();
}

// ============================================================
// CONSTRUCCION DE WIDGETS DE ITEM
// ============================================================

void UInventoryGridWidget::RebuildItemWidgets()
{
	if (!GridCanvas)
	{
		UE_LOG(LogTemp, Warning, TEXT("UInventoryGridWidget::RebuildItemWidgets — GridCanvas es nulo"));
		return;
	}

	// Limpiar todos los widgets de item anteriores
	GridCanvas->ClearChildren();
	ActiveItemWidgets.Empty();

	if (!OwningInventory || !ItemWidgetClass)
	{
		return;
	}

	const int32 Columns = OwningInventory->GridColumns;
	const TArray<FInventorySlot>& Grid = OwningInventory->Grid;

	for (const FInventorySlot& Slot : Grid)
	{
		// Solo procesar slots raíz ocupados; las celdas secundarias se ignoran
		if (!Slot.bIsOccupied || !Slot.bIsRootSlot || Slot.SlotIndex < 0)
		{
			continue;
		}

		// Calcular fila y columna a partir del índice lineal
		const int32 Col = Slot.SlotIndex % Columns;
		const int32 Row = Slot.SlotIndex / Columns;

		// Crear el widget de item
		UInventoryItemWidget* ItemWidget =
			CreateWidget<UInventoryItemWidget>(GetOwningPlayer(), ItemWidgetClass);

		if (!ItemWidget)
		{
			continue;
		}

		// Poblar datos del widget
		ItemWidget->ItemData       = Slot.ItemData;
		ItemWidget->RootSlotIndex  = Slot.SlotIndex;
		ItemWidget->bIsRotated     = Slot.bIsRotated;
		ItemWidget->CellSize       = CellSize;

		// Añadir al canvas y configurar posición/tamaño
		UCanvasPanelSlot* CanvasSlot = GridCanvas->AddChildToCanvas(ItemWidget);
		if (CanvasSlot)
		{
			const float PosX = Col * CellSize;
			const float PosY = Row * CellSize;

			const int32 EffW = Slot.GetEffectiveWidth();
			const int32 EffH = Slot.GetEffectiveHeight();

			CanvasSlot->SetPosition(FVector2D(PosX, PosY));
			CanvasSlot->SetSize(FVector2D(EffW * CellSize, EffH * CellSize));
			CanvasSlot->SetAutoSize(false);
			CanvasSlot->SetZOrder(1); // Items por encima del fondo de celdas
		}

		// Actualizar los visuales del Blueprint
		ItemWidget->RefreshVisuals(Slot.ItemData, Slot.bIsRotated);

		// Vincular delegados de interacción
		BindItemWidgetDelegates(ItemWidget);

		ActiveItemWidgets.Add(ItemWidget);
	}
}

// ============================================================
// CONSULTAS DE POSICION Y VALIDACION
// ============================================================

int32 UInventoryGridWidget::GetSlotIndexAtPosition(FVector2D LocalPosition) const
{
	if (!OwningInventory)
	{
		return -1;
	}

	// Convertir posición en píxeles a columna y fila
	const int32 Col = FMath::FloorToInt(LocalPosition.X / CellSize);
	const int32 Row = FMath::FloorToInt(LocalPosition.Y / CellSize);

	// Comprobar límites de la cuadrícula
	if (Col < 0 || Col >= OwningInventory->GridColumns ||
		Row < 0 || Row >= OwningInventory->GridRows)
	{
		return -1;
	}

	return Row * OwningInventory->GridColumns + Col;
}

bool UInventoryGridWidget::CanDropAt(
	int32 SlotIndex, int32 EffW, int32 EffH, int32 ExcludeRootSlot) const
{
	if (!OwningInventory || SlotIndex < 0)
	{
		return false;
	}

	const int32 Columns = OwningInventory->GridColumns;
	const int32 Rows    = OwningInventory->GridRows;
	const int32 StartCol = SlotIndex % Columns;
	const int32 StartRow = SlotIndex / Columns;

	// Verificar que el item cabe dentro de los límites de la cuadrícula
	if (StartCol + EffW > Columns || StartRow + EffH > Rows)
	{
		return false;
	}

	const TArray<FInventorySlot>& Grid = OwningInventory->Grid;

	// Primero encontrar el slot raíz del item excluido para ignorar sus celdas
	// Se identifica comparando el índice raíz almacenado en los datos de cada slot
	for (int32 DeltaRow = 0; DeltaRow < EffH; ++DeltaRow)
	{
		for (int32 DeltaCol = 0; DeltaCol < EffW; ++DeltaCol)
		{
			const int32 CheckIdx = (StartRow + DeltaRow) * Columns + (StartCol + DeltaCol);

			if (!Grid.IsValidIndex(CheckIdx))
			{
				return false;
			}

			const FInventorySlot& CheckSlot = Grid[CheckIdx];

			if (CheckSlot.bIsOccupied)
			{
				// Si la celda pertenece al item que estamos moviendo, la ignoramos.
				// Encontramos el slot raíz de esa celda comparando el ItemData.ID
				if (ExcludeRootSlot >= 0 && Grid.IsValidIndex(ExcludeRootSlot))
				{
					const FGuid& ExcludeID = Grid[ExcludeRootSlot].ItemData.ID;
					if (CheckSlot.ItemData.ID == ExcludeID)
					{
						// Esta celda pertenece al item que estamos arrastrando; es válida
						continue;
					}
				}
				// La celda está ocupada por otro item — no hay espacio
				return false;
			}
		}
	}

	return true;
}

// ============================================================
// VISUALIZACION DE PREVISTA DE SOLTAR
// ============================================================

void UInventoryGridWidget::ShowDropPreview(
	int32 SlotIndex, int32 EffW, int32 EffH, bool bValid)
{
	// Limpiar resaltados previos antes de mostrar los nuevos
	ClearAllHighlights();

	if (!OwningInventory || SlotIndex < 0)
	{
		return;
	}

	const int32 Columns = OwningInventory->GridColumns;
	const int32 Rows    = OwningInventory->GridRows;
	const int32 StartCol = SlotIndex % Columns;
	const int32 StartRow = SlotIndex / Columns;

	for (int32 DeltaRow = 0; DeltaRow < EffH; ++DeltaRow)
	{
		for (int32 DeltaCol = 0; DeltaCol < EffW; ++DeltaCol)
		{
			// Verificar que la celda existe en la cuadrícula antes de resaltarla
			const int32 HighlightCol = StartCol + DeltaCol;
			const int32 HighlightRow = StartRow + DeltaRow;

			if (HighlightCol >= Columns || HighlightRow >= Rows)
			{
				continue;
			}

			const int32 HighlightIdx = HighlightRow * Columns + HighlightCol;
			SetCellHighlight(HighlightIdx, true, bValid);
		}
	}

	// Resaltar también el widget de item si el cursor está sobre él
	// (le indica al jugador que el intercambio es posible)
	for (UInventoryItemWidget* ItemWidget : ActiveItemWidgets)
	{
		if (ItemWidget && ItemWidget->RootSlotIndex == SlotIndex)
		{
			ItemWidget->SetHighlight(true, bValid);
		}
	}
}

// ============================================================
// EVENTOS NATIVOS DE DRAG & DROP
// ============================================================

bool UInventoryGridWidget::NativeOnDragOver(
	const FGeometry& InGeometry,
	const FDragDropEvent& InDragDropEvent,
	UDragDropOperation* InOperation)
{
	Super::NativeOnDragOver(InGeometry, InDragDropEvent, InOperation);

	UInventoryDragDropOperation* DragOp =
		Cast<UInventoryDragDropOperation>(InOperation);

	if (!DragOp || !OwningInventory)
	{
		return false;
	}

	// Convertir posición absoluta del cursor a espacio local del canvas
	const FVector2D LocalPos = InGeometry.AbsoluteToLocal(
		InDragDropEvent.GetScreenSpacePosition());

	const int32 NewHoverSlot = GetSlotIndexAtPosition(LocalPos);
	HoverSlotIndex = NewHoverSlot;

	if (NewHoverSlot < 0)
	{
		ClearAllHighlights();
		return true;
	}

	const int32 EffW = DragOp->GetEffectiveWidth();
	const int32 EffH = DragOp->GetEffectiveHeight();
	const int32 SourceSlot = DragOp->bFromEquipment ? -1 : DragOp->SourceRootSlot;

	const bool bValid = CanDropAt(NewHoverSlot, EffW, EffH, SourceSlot);
	ShowDropPreview(NewHoverSlot, EffW, EffH, bValid);

	return true;
}

bool UInventoryGridWidget::NativeOnDrop(
	const FGeometry& InGeometry,
	const FDragDropEvent& InDragDropEvent,
	UDragDropOperation* InOperation)
{
	Super::NativeOnDrop(InGeometry, InDragDropEvent, InOperation);

	UInventoryDragDropOperation* DragOp =
		Cast<UInventoryDragDropOperation>(InOperation);

	if (!DragOp || !OwningInventory)
	{
		ClearAllHighlights();
		return false;
	}

	// Calcular el slot de destino a partir de la posición del cursor
	const FVector2D LocalPos = InGeometry.AbsoluteToLocal(
		InDragDropEvent.GetScreenSpacePosition());

	const int32 TargetSlot = GetSlotIndexAtPosition(LocalPos);
	if (TargetSlot < 0)
	{
		ClearAllHighlights();
		return false;
	}

	const int32 EffW = DragOp->GetEffectiveWidth();
	const int32 EffH = DragOp->GetEffectiveHeight();
	const int32 SourceSlot = DragOp->bFromEquipment ? -1 : DragOp->SourceRootSlot;

	// Verificar que la posición es válida antes de enviar el RPC
	if (!CanDropAt(TargetSlot, EffW, EffH, SourceSlot))
	{
		ClearAllHighlights();
		return false;
	}

	// Enviar RPC al servidor: mover item de la posición de origen a la de destino
	// bRotate refleja si el jugador pulsó R durante el arrastre
	OwningInventory->ServerRequestMoveItem(
		DragOp->SourceRootSlot,
		TargetSlot,
		DragOp->bIsRotated);

	ClearAllHighlights();
	HoverSlotIndex = -1;
	return true;
}

void UInventoryGridWidget::NativeOnDragLeave(
	const FDragDropEvent& InDragDropEvent,
	UDragDropOperation* InOperation)
{
	Super::NativeOnDragLeave(InDragDropEvent, InOperation);

	// El cursor salió de la cuadrícula — eliminar todos los resaltados
	ClearAllHighlights();

	// Quitar resaltado de todos los widgets de item activos
	for (UInventoryItemWidget* ItemWidget : ActiveItemWidgets)
	{
		if (ItemWidget)
		{
			ItemWidget->SetHighlight(false, false);
		}
	}

	HoverSlotIndex = -1;
}

FReply UInventoryGridWidget::NativeOnMouseMove(
	const FGeometry& InGeometry,
	const FPointerEvent& InMouseEvent)
{
	// Actualizar el slot bajo el cursor para que el widget padre
	// pueda posicionar el tooltip correctamente
	const FVector2D LocalPos = InGeometry.AbsoluteToLocal(
		InMouseEvent.GetScreenSpacePosition());

	HoverSlotIndex = GetSlotIndexAtPosition(LocalPos);

	return Super::NativeOnMouseMove(InGeometry, InMouseEvent);
}

// ============================================================
// CALLBACKS INTERNOS
// ============================================================

void UInventoryGridWidget::HandleInventoryChanged()
{
	// El servidor replicó cambios al inventario — reconstruir toda la cuadrícula
	RebuildItemWidgets();
}

void UInventoryGridWidget::BindItemWidgetDelegates(UInventoryItemWidget* ItemWidget)
{
	if (!ItemWidget)
	{
		return;
	}

	// Los delegados se gestionan desde UInventoryMainWidget que escucha
	// OnRightClicked, OnHoverStart y OnHoverEnd de cada widget de item.
	// Este método existe como punto de extensión para el widget padre.
	// UInventoryMainWidget sobrescribe o enlaza manualmente tras RebuildItemWidgets.
}
