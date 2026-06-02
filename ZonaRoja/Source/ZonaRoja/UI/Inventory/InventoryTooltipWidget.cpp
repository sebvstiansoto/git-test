// InventoryTooltipWidget.cpp
// Implementación del tooltip flotante del inventario.

#include "UI/Inventory/InventoryTooltipWidget.h"
#include "Engine/DataTable.h"
#include "Data/ZRTypes.h"

// ============================================================
// REFRESCO DEL TOOLTIP
// ============================================================

void UInventoryTooltipWidget::RefreshTooltip(const FItemData& Item)
{
	ItemTier = EItemTier::Common;
	TierColor = GetColorForTier(ItemTier);

	// -------------------------------------------------------
	// TODO: Integración con DataTable pendiente
	// El siguiente bloque debe reemplazarse por una búsqueda en
	// DT_ItemDefinitions usando Item.ItemDefinitionID para obtener
	// FItemDefinitionRow::DisplayName, Description, WeightGrams,
	// BaseValueCZ y Tier.
	//
	// Ejemplo de integración futura:
	//   UDataTable* ItemTable = ...; // Referencia al DataTable cargado
	//   FItemDefinitionRow* Row = ItemTable->FindRow<FItemDefinitionRow>(
	//       Item.ItemDefinitionID, TEXT("Tooltip"));
	//   if (Row)
	//   {
	//       ItemName    = Row->DisplayName;
	//       ItemDescription = Row->Description;
	//       float WeightKg = Row->WeightGrams / 1000.0f;
	//       WeightText  = FText::Format(NSLOCTEXT("ZR","Weight","{0} kg"),
	//                         FText::AsNumber(WeightKg));
	//       ValueText   = FText::Format(NSLOCTEXT("ZR","Value","{0} CZ"),
	//                         FText::AsNumber(Row->BaseValueCZ));
	//       ItemTier    = Row->Tier;
	//       TierColor   = GetColorForTier(ItemTier);
	//   }
	// -------------------------------------------------------

	// Valores de marcador de posición mientras DataTable no esté disponible
	if (Item.ItemDefinitionID != NAME_None)
	{
		// Mostrar el ID de definición como nombre provisional
		ItemName = FText::FromName(Item.ItemDefinitionID);
	}
	else
	{
		ItemName = NSLOCTEXT("ZonaRoja", "ItemDesconocido", "Objeto desconocido");
	}

	ItemDescription = NSLOCTEXT("ZonaRoja", "SinDescripcion",
		"Descripción no disponible — DataTable pendiente de integración.");

	// Peso provisional: siempre 0.0 hasta tener el DataTable
	WeightText = NSLOCTEXT("ZonaRoja", "PesoND", "? kg");

	// Valor provisional
	ValueText = NSLOCTEXT("ZonaRoja", "ValorND", "? CZ");

	// Durabilidad: -1 indica que el item no tiene durabilidad
	if (Item.Durability < 0.0f)
	{
		DurabilityText = FText::FromString(TEXT("—"));
	}
	else
	{
		// Mostrar "87 / 100" con la durabilidad redondeada al entero más cercano
		const int32 Current = FMath::RoundToInt(Item.Durability);
		DurabilityText = FText::Format(
			NSLOCTEXT("ZonaRoja", "Durabilidad", "{0} / 100"),
			FText::AsNumber(Current));
	}

	// Aplicar el color de rareza calculado
	TierColor = GetColorForTier(ItemTier);

	// Notificar al Blueprint para que actualice los elementos visuales UMG
	UpdateTooltipVisuals();
}

// ============================================================
// COLOR POR RAREZA
// ============================================================

FLinearColor UInventoryTooltipWidget::GetColorForTier(EItemTier Tier)
{
	switch (Tier)
	{
	case EItemTier::Common:
		// Gris neutro — items básicos sin valor especial
		return FLinearColor(0.5f, 0.5f, 0.5f, 1.0f);

	case EItemTier::Uncommon:
		// Verde — items de calidad mejorada
		return FLinearColor(0.1f, 0.8f, 0.1f, 1.0f);

	case EItemTier::Rare:
		// Azul — items raros con buenas estadísticas
		return FLinearColor(0.1f, 0.4f, 0.9f, 1.0f);

	case EItemTier::Epic:
		// Púrpura — items épicos de alto valor
		return FLinearColor(0.6f, 0.1f, 0.9f, 1.0f);

	case EItemTier::Legendary:
		// Naranja — items legendarios únicos o de máxima calidad
		return FLinearColor(1.0f, 0.5f, 0.0f, 1.0f);

	default:
		return FLinearColor::White;
	}
}
