// InventoryComponent.cpp
// Implementación del componente de inventario tipo cuadrícula para ZonaRoja

#include "Components/InventoryComponent.h"
#include "Net/UnrealNetwork.h"

UInventoryComponent::UInventoryComponent()
{
	SetIsReplicatedByDefault(true);

	// Cuadrícula de inventario predeterminada: 10x10 celdas (100 slots)
	// Se ampliará con accesorios como mochilas y chalecos
	GridColumns = 10;
	GridRows = 10;
}

void UInventoryComponent::BeginPlay()
{
	Super::BeginPlay();

	// Inicializar la cuadrícula solo en el servidor
	if (GetOwner()->HasAuthority())
	{
		InitializeGrid();
	}
}

void UInventoryComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(UInventoryComponent, InventoryGrid);
	DOREPLIFETIME(UInventoryComponent, EquipmentSlots);
}

// ============================================================
// INICIALIZACION
// ============================================================

void UInventoryComponent::InitializeGrid()
{
	const int32 TotalSlots = GridColumns * GridRows;
	InventoryGrid.SetNum(TotalSlots);

	for (int32 i = 0; i < TotalSlots; ++i)
	{
		InventoryGrid[i].bIsOccupied = false;
		InventoryGrid[i].SlotIndex = i;
	}
}

// ============================================================
// FUNCIONES PRINCIPALES
// ============================================================

bool UInventoryComponent::TryAddItem(const FItemData& ItemData, int32 ItemWidth, int32 ItemHeight)
{
	if (!GetOwner()->HasAuthority())
	{
		return false; // Solo el servidor puede modificar el inventario
	}

	int32 FreeSlot = -1;
	if (FindFreeSlot(ItemWidth, ItemHeight, FreeSlot))
	{
		return AddItemToSlot(ItemData, FreeSlot, ItemWidth, ItemHeight);
	}

	UE_LOG(LogTemp, Warning, TEXT("[ZonaRoja] Inventario lleno: no se pudo añadir %s"),
		*ItemData.ItemDefinitionID.ToString());
	return false;
}

bool UInventoryComponent::AddItemToSlot(const FItemData& ItemData, int32 SlotIndex,
	int32 ItemWidth, int32 ItemHeight)
{
	if (!GetOwner()->HasAuthority())
	{
		return false;
	}

	if (!CanFitItemAt(SlotIndex, ItemWidth, ItemHeight))
	{
		return false;
	}

	// Marcar todas las celdas ocupadas por el item
	const int32 StartRow = SlotIndex / GridColumns;
	const int32 StartCol = SlotIndex % GridColumns;

	for (int32 Row = StartRow; Row < StartRow + ItemHeight; ++Row)
	{
		for (int32 Col = StartCol; Col < StartCol + ItemWidth; ++Col)
		{
			const int32 CellIndex = Row * GridColumns + Col;
			if (CellIndex < InventoryGrid.Num())
			{
				InventoryGrid[CellIndex].bIsOccupied = true;
				InventoryGrid[CellIndex].ItemData = ItemData;
				InventoryGrid[CellIndex].ItemWidth = ItemWidth;
				InventoryGrid[CellIndex].ItemHeight = ItemHeight;
			}
		}
	}

	// Marcar el slot principal con los datos completos del item
	InventoryGrid[SlotIndex].ItemData = ItemData;

	OnItemAdded.Broadcast(ItemData, SlotIndex);
	OnInventoryChanged.Broadcast();

	UE_LOG(LogTemp, Log, TEXT("[ZonaRoja] Item añadido al inventario: %s en slot %d"),
		*ItemData.ItemDefinitionID.ToString(), SlotIndex);
	return true;
}

bool UInventoryComponent::RemoveItem(const FGuid& ItemID)
{
	if (!GetOwner()->HasAuthority())
	{
		return false;
	}

	for (int32 i = 0; i < InventoryGrid.Num(); ++i)
	{
		if (InventoryGrid[i].bIsOccupied && InventoryGrid[i].ItemData.ID == ItemID)
		{
			// Limpiar todas las celdas ocupadas por este item
			const FItemData RemovedItem = InventoryGrid[i].ItemData;
			const int32 Width = InventoryGrid[i].ItemWidth;
			const int32 Height = InventoryGrid[i].ItemHeight;
			const int32 StartRow = i / GridColumns;
			const int32 StartCol = i % GridColumns;

			for (int32 Row = StartRow; Row < StartRow + Height; ++Row)
			{
				for (int32 Col = StartCol; Col < StartCol + Width; ++Col)
				{
					const int32 CellIndex = Row * GridColumns + Col;
					if (CellIndex < InventoryGrid.Num())
					{
						InventoryGrid[CellIndex].bIsOccupied = false;
						InventoryGrid[CellIndex].ItemData = FItemData();
						InventoryGrid[CellIndex].ItemWidth = 1;
						InventoryGrid[CellIndex].ItemHeight = 1;
					}
				}
			}

			OnItemRemoved.Broadcast(ItemID);
			OnInventoryChanged.Broadcast();
			return true;
		}
	}

	return false;
}

int32 UInventoryComponent::RemoveItemByDefinition(const FName& ItemDefinitionID, int32 Amount)
{
	if (!GetOwner()->HasAuthority())
	{
		return 0;
	}

	int32 RemovedCount = 0;

	for (int32 i = 0; i < InventoryGrid.Num() && RemovedCount < Amount; ++i)
	{
		if (InventoryGrid[i].bIsOccupied &&
			InventoryGrid[i].ItemData.ItemDefinitionID == ItemDefinitionID)
		{
			const int32 StackCount = InventoryGrid[i].ItemData.StackCount;
			const int32 ToRemove = FMath::Min(StackCount, Amount - RemovedCount);

			if (ToRemove >= StackCount)
			{
				// Eliminar todo el stack
				RemoveItem(InventoryGrid[i].ItemData.ID);
				RemovedCount += StackCount;
			}
			else
			{
				// Reducir el stack
				InventoryGrid[i].ItemData.StackCount -= ToRemove;
				RemovedCount += ToRemove;
				OnInventoryChanged.Broadcast();
			}
		}
	}

	return RemovedCount;
}

bool UInventoryComponent::EquipItem(const FGuid& ItemID, EEquipmentSlot Slot)
{
	if (!GetOwner()->HasAuthority())
	{
		return false;
	}

	// Buscar el item en el inventario
	FItemData ItemToEquip;
	if (!FindItem(ItemID, ItemToEquip))
	{
		return false;
	}

	// Si la ranura ya tiene un item, intercambiarlos
	FItemData* CurrentEquipped = nullptr;
	switch (Slot)
	{
	case EEquipmentSlot::PrimaryWeapon:   CurrentEquipped = &EquipmentSlots.PrimaryWeapon;   break;
	case EEquipmentSlot::SecondaryWeapon: CurrentEquipped = &EquipmentSlots.SecondaryWeapon; break;
	case EEquipmentSlot::Holster:         CurrentEquipped = &EquipmentSlots.Holster;         break;
	case EEquipmentSlot::Helmet:          CurrentEquipped = &EquipmentSlots.Helmet;          break;
	case EEquipmentSlot::BodyArmor:       CurrentEquipped = &EquipmentSlots.BodyArmor;       break;
	case EEquipmentSlot::Vest:            CurrentEquipped = &EquipmentSlots.Vest;            break;
	case EEquipmentSlot::Backpack:        CurrentEquipped = &EquipmentSlots.Backpack;        break;
	case EEquipmentSlot::LeftPocket:      CurrentEquipped = &EquipmentSlots.LeftPocket;      break;
	case EEquipmentSlot::RightPocket:     CurrentEquipped = &EquipmentSlots.RightPocket;     break;
	default: return false;
	}

	// Si hay algo equipado, volver al inventario
	if (CurrentEquipped && CurrentEquipped->IsValid())
	{
		TryAddItem(*CurrentEquipped);
	}

	// Equipar el nuevo item
	*CurrentEquipped = ItemToEquip;

	// Quitar del inventario
	RemoveItem(ItemID);

	OnEquipmentChanged.Broadcast(Slot, ItemToEquip);
	return true;
}

bool UInventoryComponent::UnequipItem(EEquipmentSlot Slot)
{
	if (!GetOwner()->HasAuthority())
	{
		return false;
	}

	FItemData* CurrentEquipped = nullptr;
	switch (Slot)
	{
	case EEquipmentSlot::PrimaryWeapon:   CurrentEquipped = &EquipmentSlots.PrimaryWeapon;   break;
	case EEquipmentSlot::SecondaryWeapon: CurrentEquipped = &EquipmentSlots.SecondaryWeapon; break;
	case EEquipmentSlot::Holster:         CurrentEquipped = &EquipmentSlots.Holster;         break;
	case EEquipmentSlot::Helmet:          CurrentEquipped = &EquipmentSlots.Helmet;          break;
	case EEquipmentSlot::BodyArmor:       CurrentEquipped = &EquipmentSlots.BodyArmor;       break;
	case EEquipmentSlot::Vest:            CurrentEquipped = &EquipmentSlots.Vest;            break;
	case EEquipmentSlot::Backpack:        CurrentEquipped = &EquipmentSlots.Backpack;        break;
	case EEquipmentSlot::LeftPocket:      CurrentEquipped = &EquipmentSlots.LeftPocket;      break;
	case EEquipmentSlot::RightPocket:     CurrentEquipped = &EquipmentSlots.RightPocket;     break;
	default: return false;
	}

	if (!CurrentEquipped || !CurrentEquipped->IsValid())
	{
		return false; // La ranura está vacía
	}

	if (TryAddItem(*CurrentEquipped))
	{
		*CurrentEquipped = FItemData(); // Vaciar la ranura
		OnEquipmentChanged.Broadcast(Slot, FItemData());
		return true;
	}

	return false; // No hay espacio en el inventario
}

bool UInventoryComponent::MoveItem(int32 FromSlot, int32 ToSlot)
{
	if (!GetOwner()->HasAuthority())
	{
		return false;
	}

	if (FromSlot < 0 || FromSlot >= InventoryGrid.Num() ||
		ToSlot < 0 || ToSlot >= InventoryGrid.Num())
	{
		return false;
	}

	if (!InventoryGrid[FromSlot].bIsOccupied)
	{
		return false; // El slot de origen está vacío
	}

	// Guardar datos del item origen
	const FInventorySlot SourceSlot = InventoryGrid[FromSlot];

	// Verificar si el destino puede contener el item
	if (!CanFitItemAt(ToSlot, SourceSlot.ItemWidth, SourceSlot.ItemHeight))
	{
		return false;
	}

	// Eliminar del origen y añadir al destino
	RemoveItem(SourceSlot.ItemData.ID);
	return AddItemToSlot(SourceSlot.ItemData, ToSlot, SourceSlot.ItemWidth, SourceSlot.ItemHeight);
}

// ============================================================
// CONSULTAS
// ============================================================

bool UInventoryComponent::FindItem(const FGuid& ItemID, FItemData& OutItemData) const
{
	for (const FInventorySlot& Slot : InventoryGrid)
	{
		if (Slot.bIsOccupied && Slot.ItemData.ID == ItemID)
		{
			OutItemData = Slot.ItemData;
			return true;
		}
	}
	return false;
}

int32 UInventoryComponent::GetItemCount(const FName& ItemDefinitionID) const
{
	int32 Count = 0;
	// Evitar contar slots que son "continuación" del mismo item
	TSet<FGuid> CountedIDs;

	for (const FInventorySlot& Slot : InventoryGrid)
	{
		if (Slot.bIsOccupied &&
			Slot.ItemData.ItemDefinitionID == ItemDefinitionID &&
			!CountedIDs.Contains(Slot.ItemData.ID))
		{
			Count += Slot.ItemData.StackCount;
			CountedIDs.Add(Slot.ItemData.ID);
		}
	}
	return Count;
}

bool UInventoryComponent::HasSpaceFor(int32 ItemWidth, int32 ItemHeight) const
{
	int32 IgnoredSlot = -1;
	return FindFreeSlot(ItemWidth, ItemHeight, IgnoredSlot);
}

bool UInventoryComponent::GetEquippedItem(EEquipmentSlot Slot, FItemData& OutItemData) const
{
	switch (Slot)
	{
	case EEquipmentSlot::PrimaryWeapon:   OutItemData = EquipmentSlots.PrimaryWeapon;   break;
	case EEquipmentSlot::SecondaryWeapon: OutItemData = EquipmentSlots.SecondaryWeapon; break;
	case EEquipmentSlot::Holster:         OutItemData = EquipmentSlots.Holster;         break;
	case EEquipmentSlot::Helmet:          OutItemData = EquipmentSlots.Helmet;          break;
	case EEquipmentSlot::BodyArmor:       OutItemData = EquipmentSlots.BodyArmor;       break;
	case EEquipmentSlot::Vest:            OutItemData = EquipmentSlots.Vest;            break;
	case EEquipmentSlot::Backpack:        OutItemData = EquipmentSlots.Backpack;        break;
	case EEquipmentSlot::LeftPocket:      OutItemData = EquipmentSlots.LeftPocket;      break;
	case EEquipmentSlot::RightPocket:     OutItemData = EquipmentSlots.RightPocket;     break;
	default: return false;
	}
	return OutItemData.IsValid();
}

int32 UInventoryComponent::GetUsedSlotCount() const
{
	int32 UsedCount = 0;
	for (const FInventorySlot& Slot : InventoryGrid)
	{
		if (Slot.bIsOccupied)
		{
			UsedCount++;
		}
	}
	return UsedCount;
}

// ============================================================
// UTILIDADES INTERNAS
// ============================================================

bool UInventoryComponent::FindFreeSlot(int32 ItemWidth, int32 ItemHeight, int32& OutSlotIndex) const
{
	for (int32 Row = 0; Row <= GridRows - ItemHeight; ++Row)
	{
		for (int32 Col = 0; Col <= GridColumns - ItemWidth; ++Col)
		{
			const int32 SlotIndex = Row * GridColumns + Col;
			if (CanFitItemAt(SlotIndex, ItemWidth, ItemHeight))
			{
				OutSlotIndex = SlotIndex;
				return true;
			}
		}
	}
	return false;
}

bool UInventoryComponent::CanFitItemAt(int32 StartSlot, int32 ItemWidth, int32 ItemHeight) const
{
	if (StartSlot < 0 || StartSlot >= InventoryGrid.Num())
	{
		return false;
	}

	const int32 StartRow = StartSlot / GridColumns;
	const int32 StartCol = StartSlot % GridColumns;

	// Verificar que el item no se salga de los límites de la cuadrícula
	if (StartCol + ItemWidth > GridColumns || StartRow + ItemHeight > GridRows)
	{
		return false;
	}

	// Verificar que todas las celdas necesarias estén libres
	for (int32 Row = StartRow; Row < StartRow + ItemHeight; ++Row)
	{
		for (int32 Col = StartCol; Col < StartCol + ItemWidth; ++Col)
		{
			const int32 CellIndex = Row * GridColumns + Col;
			if (CellIndex >= InventoryGrid.Num() || InventoryGrid[CellIndex].bIsOccupied)
			{
				return false;
			}
		}
	}

	return true;
}

// ============================================================
// CALLBACKS DE REPLICACION
// ============================================================

void UInventoryComponent::OnRep_Inventory()
{
	// Notificar a la UI que el inventario cambió en el cliente
	OnInventoryChanged.Broadcast();
}

void UInventoryComponent::OnRep_Equipment()
{
	// La UI de equipamiento se actualiza en el Blueprint del personaje
	OnInventoryChanged.Broadcast();
}
