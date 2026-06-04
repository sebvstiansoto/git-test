// InventoryComponent.cpp

#include "Components/InventoryComponent.h"
#include "Net/UnrealNetwork.h"
#include "Engine/DataTable.h"
#include "GameFramework/Actor.h"

// DataTable de definiciones de items (asignado en el GameInstance o Config DataAsset)
static UDataTable* GItemDefinitionTable = nullptr;

// ============================================================
// CONSTRUCTOR
// ============================================================

UInventoryComponent::UInventoryComponent()
{
	SetIsReplicatedByDefault(true);
	PrimaryComponentTick.bCanEverTick = false;

	GridColumns = BaseGridColumns;
	GridRows    = BaseGridRows;
}

void UInventoryComponent::BeginPlay()
{
	Super::BeginPlay();

	if (GetOwner()->HasAuthority())
	{
		RebuildGrid();
	}
}

void UInventoryComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	// El grid y el equipo solo van al dueño (anti-cheat: otros jugadores no deben
	// saber el contenido exacto del inventario de un rival)
	DOREPLIFETIME_CONDITION(UInventoryComponent, Grid,             COND_OwnerOnly);
	DOREPLIFETIME_CONDITION(UInventoryComponent, Equipment,        COND_OwnerOnly);
	DOREPLIFETIME_CONDITION(UInventoryComponent, CurrentWeightGrams, COND_OwnerOnly);
	DOREPLIFETIME(UInventoryComponent, GridColumns);
	DOREPLIFETIME(UInventoryComponent, GridRows);
}

// ============================================================
// OPERACIONES PRINCIPALES
// ============================================================

EInventoryResult UInventoryComponent::TryAddItem(const FItemData& ItemData,
	int32 ItemWidth, int32 ItemHeight)
{
	if (!GetOwner()->HasAuthority()) return EInventoryResult::NotAuthority;
	if (!ItemData.IsValid())         return EInventoryResult::InvalidItem;

	// Intentar apilar sobre stacks existentes del mismo tipo
	if (ItemData.StackCount > 1 || ItemWidth == 1)
	{
		for (FInventorySlot& Slot : Grid)
		{
			if (!Slot.bIsRootSlot) continue;
			if (Slot.ItemData.ItemDefinitionID != ItemData.ItemDefinitionID) continue;

			// TODO: consultar MaxStackSize desde DT_ItemDefinitions
			// Por ahora usamos 999 como máximo universal para apilables
			const int32 MaxStack = 999;
			const int32 Remaining = MaxStack - Slot.ItemData.StackCount;
			if (Remaining <= 0) continue;

			const int32 ToAdd = FMath::Min(ItemData.StackCount, Remaining);
			Slot.ItemData.StackCount += ToAdd;

			// Si se apiló todo, notificar y salir
			if (ToAdd >= ItemData.StackCount)
			{
				OnItemAdded.Broadcast(Slot.ItemData, Slot.SlotIndex);
				OnInventoryChanged.Broadcast();
				RecalculateWeight();
				return EInventoryResult::Success;
			}
		}
	}

	// Buscar slot libre (primero sin rotar, luego rotado)
	int32 FreeSlot = FindFirstFreeSlot(ItemWidth, ItemHeight, false);
	bool  bRotated = false;

	if (FreeSlot == -1 && ItemWidth != ItemHeight)
	{
		FreeSlot = FindFirstFreeSlot(ItemHeight, ItemWidth, false);
		bRotated  = (FreeSlot != -1);
	}

	if (FreeSlot == -1)
	{
		UE_LOG(LogTemp, Warning,
			TEXT("[Inventario] Sin espacio para: %s (%dx%d)"),
			*ItemData.ItemDefinitionID.ToString(), ItemWidth, ItemHeight);
		return EInventoryResult::NoSpace;
	}

	return PlaceItemAt(ItemData, FreeSlot, ItemWidth, ItemHeight, bRotated);
}

EInventoryResult UInventoryComponent::PlaceItemAt(const FItemData& ItemData, int32 SlotIndex,
	int32 ItemWidth, int32 ItemHeight, bool bRotated)
{
	if (!GetOwner()->HasAuthority()) return EInventoryResult::NotAuthority;
	if (!ItemData.IsValid())         return EInventoryResult::InvalidItem;
	if (SlotIndex < 0 || SlotIndex >= Grid.Num()) return EInventoryResult::InvalidSlot;

	const int32 EffW = bRotated ? ItemHeight : ItemWidth;
	const int32 EffH = bRotated ? ItemWidth  : ItemHeight;

	if (!CanFitAt(SlotIndex, EffW, EffH)) return EInventoryResult::SlotOccupied;

	OccupyCells(SlotIndex, ItemData, EffW, EffH, bRotated, true);

	RecalculateWeight();
	OnItemAdded.Broadcast(ItemData, SlotIndex);
	OnInventoryChanged.Broadcast();
	return EInventoryResult::Success;
}

EInventoryResult UInventoryComponent::RemoveItem(const FGuid& ItemID)
{
	if (!GetOwner()->HasAuthority()) return EInventoryResult::NotAuthority;

	// Solo buscar en slots raíz para evitar doble-borrado
	for (int32 i = 0; i < Grid.Num(); ++i)
	{
		if (!Grid[i].bIsRootSlot)              continue;
		if (Grid[i].ItemData.ID != ItemID)     continue;

		const int32 EffW = Grid[i].GetEffectiveWidth();
		const int32 EffH = Grid[i].GetEffectiveHeight();

		OccupyCells(i, Grid[i].ItemData, EffW, EffH, Grid[i].bIsRotated, false);

		RecalculateWeight();
		OnItemRemoved.Broadcast(ItemID);
		OnInventoryChanged.Broadcast();
		return EInventoryResult::Success;
	}

	return EInventoryResult::ItemNotFound;
}

EInventoryResult UInventoryComponent::MoveItem(int32 FromRootSlot, int32 ToRootSlot,
	bool bRotateOnMove)
{
	if (!GetOwner()->HasAuthority()) return EInventoryResult::NotAuthority;
	if (FromRootSlot == ToRootSlot)  return EInventoryResult::Success;

	if (FromRootSlot < 0 || FromRootSlot >= Grid.Num() ||
		ToRootSlot   < 0 || ToRootSlot   >= Grid.Num())
		return EInventoryResult::InvalidSlot;

	if (!Grid[FromRootSlot].bIsRootSlot)
		return EInventoryResult::InvalidSlot;

	const FInventorySlot SourceCopy = Grid[FromRootSlot];
	const bool   NewRotation = SourceCopy.bIsRotated ^ bRotateOnMove;
	const int32  NewEffW     = NewRotation ? SourceCopy.ItemHeight : SourceCopy.ItemWidth;
	const int32  NewEffH     = NewRotation ? SourceCopy.ItemWidth  : SourceCopy.ItemHeight;

	// Liberar celdas originales temporalmente
	OccupyCells(FromRootSlot, SourceCopy.ItemData,
		SourceCopy.GetEffectiveWidth(), SourceCopy.GetEffectiveHeight(),
		SourceCopy.bIsRotated, false);

	// Caso intercambio: el destino tiene otro item
	if (Grid[ToRootSlot].bIsRootSlot)
	{
		const FInventorySlot DestCopy = Grid[ToRootSlot];

		OccupyCells(ToRootSlot, DestCopy.ItemData,
			DestCopy.GetEffectiveWidth(), DestCopy.GetEffectiveHeight(),
			DestCopy.bIsRotated, false);

		// Intentar colocar el item destino donde estaba el origen
		if (!CanFitAt(FromRootSlot, DestCopy.GetEffectiveWidth(), DestCopy.GetEffectiveHeight()))
		{
			// Restaurar todo si no caben en swap
			OccupyCells(FromRootSlot, SourceCopy.ItemData,
				SourceCopy.GetEffectiveWidth(), SourceCopy.GetEffectiveHeight(),
				SourceCopy.bIsRotated, true);
			OccupyCells(ToRootSlot, DestCopy.ItemData,
				DestCopy.GetEffectiveWidth(), DestCopy.GetEffectiveHeight(),
				DestCopy.bIsRotated, true);
			return EInventoryResult::SlotOccupied;
		}

		OccupyCells(FromRootSlot, DestCopy.ItemData,
			DestCopy.GetEffectiveWidth(), DestCopy.GetEffectiveHeight(),
			DestCopy.bIsRotated, true);
	}

	// Colocar el item origen en el destino
	if (!CanFitAt(ToRootSlot, NewEffW, NewEffH))
	{
		// Restaurar origen
		OccupyCells(FromRootSlot, SourceCopy.ItemData,
			SourceCopy.GetEffectiveWidth(), SourceCopy.GetEffectiveHeight(),
			SourceCopy.bIsRotated, true);
		return EInventoryResult::SlotOccupied;
	}

	OccupyCells(ToRootSlot, SourceCopy.ItemData, NewEffW, NewEffH, NewRotation, true);

	OnItemMoved.Broadcast(SourceCopy.ItemData.ID, ToRootSlot);
	OnInventoryChanged.Broadcast();
	return EInventoryResult::Success;
}

EInventoryResult UInventoryComponent::SplitStack(const FGuid& ItemID, int32 Amount)
{
	if (!GetOwner()->HasAuthority()) return EInventoryResult::NotAuthority;

	const int32 RootSlot = FindItemRootSlot(ItemID);
	if (RootSlot == -1) return EInventoryResult::ItemNotFound;

	FInventorySlot& Slot = Grid[RootSlot];
	if (Slot.ItemData.StackCount <= 1)  return EInventoryResult::CannotStack;
	if (Amount <= 0 || Amount >= Slot.ItemData.StackCount)
		return EInventoryResult::InvalidItem;

	// Crear nuevo item con la cantidad dividida
	FItemData SplitItem(Slot.ItemData.ItemDefinitionID, Slot.ItemData.StackCount - Amount);
	SplitItem.Durability = Slot.ItemData.Durability;

	// Reducir el stack original
	Slot.ItemData.StackCount = Amount;

	// Añadir el nuevo stack al inventario
	return TryAddItem(SplitItem, Slot.ItemWidth, Slot.ItemHeight);
}

int32 UInventoryComponent::RemoveAmountByDefinition(const FName& ItemDefinitionID, int32 Amount)
{
	if (!GetOwner()->HasAuthority()) return 0;

	int32 Remaining = Amount;

	// Ordenar slots por tamaño de stack ascendente para reducir fragmentación
	TArray<int32> RootSlots;
	for (int32 i = 0; i < Grid.Num(); ++i)
	{
		if (Grid[i].bIsRootSlot && Grid[i].ItemData.ItemDefinitionID == ItemDefinitionID)
			RootSlots.Add(i);
	}
	RootSlots.Sort([this](int32 A, int32 B)
	{
		return Grid[A].ItemData.StackCount < Grid[B].ItemData.StackCount;
	});

	for (int32 SlotIdx : RootSlots)
	{
		if (Remaining <= 0) break;

		FInventorySlot& Slot = Grid[SlotIdx];
		const int32 ToRemove = FMath::Min(Slot.ItemData.StackCount, Remaining);

		if (ToRemove >= Slot.ItemData.StackCount)
		{
			const FGuid RemovedID = Slot.ItemData.ID;
			OccupyCells(SlotIdx, Slot.ItemData,
				Slot.GetEffectiveWidth(), Slot.GetEffectiveHeight(),
				Slot.bIsRotated, false);
			OnItemRemoved.Broadcast(RemovedID);
		}
		else
		{
			Slot.ItemData.StackCount -= ToRemove;
		}

		Remaining -= ToRemove;
	}

	const int32 Removed = Amount - Remaining;
	if (Removed > 0)
	{
		RecalculateWeight();
		OnInventoryChanged.Broadcast();
	}
	return Removed;
}

EInventoryResult UInventoryComponent::EquipItem(const FGuid& ItemID, EEquipmentSlot Slot)
{
	if (!GetOwner()->HasAuthority()) return EInventoryResult::NotAuthority;

	FItemData ItemToEquip;
	if (!FindItem(ItemID, ItemToEquip)) return EInventoryResult::ItemNotFound;

	FItemData* EquipSlotPtr = GetEquipmentSlotPtr(Slot);
	if (!EquipSlotPtr) return EInventoryResult::InvalidSlot;

	// Si la ranura ya tiene algo, devolver al inventario
	if (EquipSlotPtr->IsValid())
	{
		const FItemData OldItem = *EquipSlotPtr;
		*EquipSlotPtr = FItemData(); // Vaciar antes de intentar añadir

		// Si no hay espacio de vuelta, abortar
		const EInventoryResult AddResult = TryAddItem(OldItem, 1, 1);
		if (AddResult != EInventoryResult::Success)
		{
			*EquipSlotPtr = OldItem; // Restaurar
			return EInventoryResult::NoSpace;
		}
	}

	RemoveItem(ItemID);
	*EquipSlotPtr = ItemToEquip;

	// Si es mochila o chaleco, recalcular tamaño del grid
	if (Slot == EEquipmentSlot::Backpack || Slot == EEquipmentSlot::Vest)
	{
		RecalculateGridSize();
	}

	RecalculateWeight();
	OnEquipmentChanged.Broadcast(Slot, ItemToEquip);
	OnInventoryChanged.Broadcast();
	return EInventoryResult::Success;
}

EInventoryResult UInventoryComponent::UnequipItem(EEquipmentSlot Slot)
{
	if (!GetOwner()->HasAuthority()) return EInventoryResult::NotAuthority;

	FItemData* EquipSlotPtr = GetEquipmentSlotPtr(Slot);
	if (!EquipSlotPtr || !EquipSlotPtr->IsValid()) return EInventoryResult::ItemNotFound;

	const FItemData ItemToReturn = *EquipSlotPtr;
	const EInventoryResult Result = TryAddItem(ItemToReturn, 1, 1);

	if (Result != EInventoryResult::Success) return EInventoryResult::NoSpace;

	*EquipSlotPtr = FItemData();

	if (Slot == EEquipmentSlot::Backpack || Slot == EEquipmentSlot::Vest)
	{
		RecalculateGridSize();
	}

	RecalculateWeight();
	OnEquipmentChanged.Broadcast(Slot, FItemData());
	OnInventoryChanged.Broadcast();
	return EInventoryResult::Success;
}

void UInventoryComponent::AutoSort()
{
	if (!GetOwner()->HasAuthority()) return;

	// Extraer todos los items con sus dimensiones
	struct FSortEntry { FItemData Item; int32 W; int32 H; };
	TArray<FSortEntry> Items;

	for (int32 i = 0; i < Grid.Num(); ++i)
	{
		if (!Grid[i].bIsRootSlot) continue;
		Items.Add({ Grid[i].ItemData, Grid[i].GetEffectiveWidth(), Grid[i].GetEffectiveHeight() });
	}

	// Ordenar por área descendente (items grandes primero)
	Items.Sort([](const FSortEntry& A, const FSortEntry& B)
	{
		return (A.W * A.H) > (B.W * B.H);
	});

	// Limpiar el grid y recolocar en orden
	RebuildGrid();

	for (const FSortEntry& Entry : Items)
	{
		TryAddItem(Entry.Item, Entry.W, Entry.H);
	}

	OnInventoryChanged.Broadcast();
}

// ============================================================
// RPCS CLIENTE → SERVIDOR
// ============================================================

void UInventoryComponent::ServerRequestMoveItem_Implementation(
	int32 FromRootSlot, int32 ToRootSlot, bool bRotate)
{
	MoveItem(FromRootSlot, ToRootSlot, bRotate);
}

void UInventoryComponent::ServerRequestDropItem_Implementation(FGuid ItemID)
{
	// TODO: hacer spawn del WorldItem en el suelo antes de eliminar
	RemoveItem(ItemID);
}

void UInventoryComponent::ServerRequestSplitStack_Implementation(FGuid ItemID, int32 Amount)
{
	SplitStack(ItemID, Amount);
}

void UInventoryComponent::ServerRequestEquipItem_Implementation(FGuid ItemID, EEquipmentSlot Slot)
{
	EquipItem(ItemID, Slot);
}

void UInventoryComponent::ServerRequestUnequipItem_Implementation(EEquipmentSlot Slot)
{
	UnequipItem(Slot);
}

void UInventoryComponent::ServerRequestAutoSort_Implementation()
{
	AutoSort();
}

// ============================================================
// CONSULTAS
// ============================================================

bool UInventoryComponent::FindItem(const FGuid& ItemID, FItemData& OutItemData) const
{
	for (const FInventorySlot& Slot : Grid)
	{
		if (Slot.bIsRootSlot && Slot.ItemData.ID == ItemID)
		{
			OutItemData = Slot.ItemData;
			return true;
		}
	}
	return false;
}

int32 UInventoryComponent::FindItemRootSlot(const FGuid& ItemID) const
{
	for (int32 i = 0; i < Grid.Num(); ++i)
	{
		if (Grid[i].bIsRootSlot && Grid[i].ItemData.ID == ItemID)
			return i;
	}
	return -1;
}

int32 UInventoryComponent::GetItemCount(const FName& ItemDefinitionID) const
{
	int32 Total = 0;
	for (const FInventorySlot& Slot : Grid)
	{
		if (Slot.bIsRootSlot && Slot.ItemData.ItemDefinitionID == ItemDefinitionID)
			Total += Slot.ItemData.StackCount;
	}
	return Total;
}

bool UInventoryComponent::HasSpaceFor(int32 ItemWidth, int32 ItemHeight) const
{
	return FindFirstFreeSlot(ItemWidth, ItemHeight, false) != -1
		|| FindFirstFreeSlot(ItemHeight, ItemWidth, false) != -1;
}

bool UInventoryComponent::GetEquippedItem(EEquipmentSlot Slot, FItemData& OutItemData) const
{
	const FItemData* Ptr = GetEquipmentSlotPtr(Slot);
	if (!Ptr) return false;
	OutItemData = *Ptr;
	return Ptr->IsValid();
}

float UInventoryComponent::GetWeightRatio() const
{
	const int32 MaxW = GetMaxWeightGrams();
	return (MaxW > 0) ? static_cast<float>(CurrentWeightGrams) / MaxW : 0.0f;
}

int32 UInventoryComponent::GetFreeSlotCount() const
{
	int32 Free = 0;
	for (const FInventorySlot& Slot : Grid)
	{
		if (!Slot.bIsOccupied) ++Free;
	}
	return Free;
}

TArray<FItemData> UInventoryComponent::GetAllItems() const
{
	TArray<FItemData> Result;
	for (const FInventorySlot& Slot : Grid)
	{
		if (Slot.bIsRootSlot)
			Result.Add(Slot.ItemData);
	}
	return Result;
}

int32 UInventoryComponent::GetTotalLootValueCZ() const
{
	// TODO: multiplicar StackCount x BaseValueCZ de DT_ItemDefinitions por cada item raíz
	return 0;
}

// ============================================================
// UTILIDADES INTERNAS
// ============================================================

void UInventoryComponent::RebuildGrid()
{
	const int32 TotalSlots = GridColumns * GridRows;
	Grid.SetNum(TotalSlots);

	for (int32 i = 0; i < TotalSlots; ++i)
	{
		Grid[i].bIsOccupied  = false;
		Grid[i].bIsRootSlot  = false;
		Grid[i].bIsRotated   = false;
		Grid[i].SlotIndex    = i;
		Grid[i].ItemData     = FItemData();
		Grid[i].ItemWidth    = 1;
		Grid[i].ItemHeight   = 1;
	}
}

void UInventoryComponent::RecalculateGridSize()
{
	int32 NewColumns = BaseGridColumns;
	int32 NewRows    = BaseGridRows;

	// Chaleco añade columnas extra
	// TODO: consultar ContainerColumns/ContainerRows desde DT_ItemDefinitions
	// Por ahora valores de ejemplo hardcodeados hasta integrar DataTable
	if (Equipment.Vest.IsValid())
	{
		NewColumns += 2; // Chaleco táctico estándar añade 2 columnas
		NewRows    += 1;
	}
	if (Equipment.Backpack.IsValid())
	{
		NewColumns += 3; // Mochila estándar añade 3 columnas y 4 filas
		NewRows    += 4;
	}

	if (NewColumns == GridColumns && NewRows == GridRows) return;

	// Guardar items actuales para recolocarlos
	TArray<FItemData> SavedItems;
	TArray<int32>     SavedWidths, SavedHeights;

	for (int32 i = 0; i < Grid.Num(); ++i)
	{
		if (!Grid[i].bIsRootSlot) continue;
		SavedItems.Add(Grid[i].ItemData);
		SavedWidths.Add(Grid[i].ItemWidth);
		SavedHeights.Add(Grid[i].ItemHeight);
	}

	GridColumns = NewColumns;
	GridRows    = NewRows;
	RebuildGrid();

	for (int32 j = 0; j < SavedItems.Num(); ++j)
	{
		TryAddItem(SavedItems[j], SavedWidths[j], SavedHeights[j]);
	}
}

void UInventoryComponent::RecalculateWeight()
{
	int32 TotalGrams = 0;

	for (const FInventorySlot& Slot : Grid)
	{
		if (!Slot.bIsRootSlot) continue;
		// TODO: leer WeightGrams * StackCount desde DT_ItemDefinitions
		// Por ahora usamos un peso fijo de 500g por item hasta integrar DataTable
		TotalGrams += 500 * Slot.ItemData.StackCount;
	}

	// Sumar peso del equipo corporal
	auto AddEquipWeight = [&](const FItemData& Item)
	{
		if (Item.IsValid()) TotalGrams += 500; // placeholder
	};
	AddEquipWeight(Equipment.Helmet);
	AddEquipWeight(Equipment.BodyArmor);
	AddEquipWeight(Equipment.Vest);
	AddEquipWeight(Equipment.Backpack);
	AddEquipWeight(Equipment.PrimaryWeapon);
	AddEquipWeight(Equipment.SecondaryWeapon);
	AddEquipWeight(Equipment.Holster);

	CurrentWeightGrams = TotalGrams;
	OnWeightChanged.Broadcast(GetCurrentWeightKg());
}

int32 UInventoryComponent::GetMaxWeightGrams() const
{
	int32 Max = BaseMaxWeightGrams;
	// TODO: añadir modificadores de habilidades (Fuerza, etc.)
	return Max;
}

int32 UInventoryComponent::FindFirstFreeSlot(int32 EffW, int32 EffH, bool /*bTryRotated*/) const
{
	for (int32 Row = 0; Row <= GridRows - EffH; ++Row)
	{
		for (int32 Col = 0; Col <= GridColumns - EffW; ++Col)
		{
			const int32 SlotIndex = Row * GridColumns + Col;
			if (CanFitAt(SlotIndex, EffW, EffH))
				return SlotIndex;
		}
	}
	return -1;
}

bool UInventoryComponent::CanFitAt(int32 SlotIndex, int32 EffW, int32 EffH) const
{
	if (SlotIndex < 0 || SlotIndex >= Grid.Num()) return false;

	const int32 StartRow = SlotIndex / GridColumns;
	const int32 StartCol = SlotIndex % GridColumns;

	if (StartCol + EffW > GridColumns) return false;
	if (StartRow + EffH > GridRows)    return false;

	for (int32 Row = StartRow; Row < StartRow + EffH; ++Row)
	{
		for (int32 Col = StartCol; Col < StartCol + EffW; ++Col)
		{
			const int32 Cell = Row * GridColumns + Col;
			if (Cell >= Grid.Num() || Grid[Cell].bIsOccupied) return false;
		}
	}
	return true;
}

void UInventoryComponent::OccupyCells(int32 RootSlot, const FItemData& ItemData,
	int32 EffW, int32 EffH, bool bRotated, bool bOccupy)
{
	const int32 StartRow = RootSlot / GridColumns;
	const int32 StartCol = RootSlot % GridColumns;

	for (int32 Row = StartRow; Row < StartRow + EffH; ++Row)
	{
		for (int32 Col = StartCol; Col < StartCol + EffW; ++Col)
		{
			const int32 Cell = Row * GridColumns + Col;
			if (Cell < 0 || Cell >= Grid.Num()) continue;

			if (bOccupy)
			{
				Grid[Cell].bIsOccupied = true;
				Grid[Cell].ItemData    = ItemData;
				Grid[Cell].ItemWidth   = bRotated ? EffH : EffW;
				Grid[Cell].ItemHeight  = bRotated ? EffW : EffH;
				Grid[Cell].bIsRotated  = bRotated;
				Grid[Cell].bIsRootSlot = (Cell == RootSlot);
			}
			else
			{
				Grid[Cell].bIsOccupied = false;
				Grid[Cell].bIsRootSlot = false;
				Grid[Cell].bIsRotated  = false;
				Grid[Cell].ItemData    = FItemData();
				Grid[Cell].ItemWidth   = 1;
				Grid[Cell].ItemHeight  = 1;
			}
		}
	}
}

FItemData* UInventoryComponent::GetEquipmentSlotPtr(EEquipmentSlot Slot)
{
	switch (Slot)
	{
	case EEquipmentSlot::PrimaryWeapon:   return &Equipment.PrimaryWeapon;
	case EEquipmentSlot::SecondaryWeapon: return &Equipment.SecondaryWeapon;
	case EEquipmentSlot::Holster:         return &Equipment.Holster;
	case EEquipmentSlot::Helmet:          return &Equipment.Helmet;
	case EEquipmentSlot::BodyArmor:       return &Equipment.BodyArmor;
	case EEquipmentSlot::Vest:            return &Equipment.Vest;
	case EEquipmentSlot::Backpack:        return &Equipment.Backpack;
	case EEquipmentSlot::LeftPocket:      return &Equipment.LeftPocket;
	case EEquipmentSlot::RightPocket:     return &Equipment.RightPocket;
	default:                              return nullptr;
	}
}

const FItemData* UInventoryComponent::GetEquipmentSlotPtr(EEquipmentSlot Slot) const
{
	return const_cast<UInventoryComponent*>(this)->GetEquipmentSlotPtr(Slot);
}

// ============================================================
// CALLBACKS DE REPLICACIÓN
// ============================================================

void UInventoryComponent::OnRep_Grid()
{
	OnInventoryChanged.Broadcast();
}

void UInventoryComponent::OnRep_Equipment()
{
	OnInventoryChanged.Broadcast();
}

void UInventoryComponent::OnRep_Weight()
{
	OnWeightChanged.Broadcast(GetCurrentWeightKg());
}
