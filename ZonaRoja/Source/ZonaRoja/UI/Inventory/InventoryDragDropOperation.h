// InventoryDragDropOperation.h
// Operación de arrastrar y soltar para items del inventario.
// Almacena todos los datos necesarios durante el arrastre de un item.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/DragDropOperation.h"
#include "Data/ZRTypes.h"
#include "InventoryDragDropOperation.generated.h"

/**
 * UInventoryDragDropOperation
 *
 * Operación de arrastre personalizada para el sistema de inventario de ZonaRoja.
 * Transporta los datos del item arrastrado, su origen y el estado de rotación.
 * Se crea al iniciar el arrastre desde UInventoryItemWidget o UEquipmentSlotWidget.
 */
UCLASS(BlueprintType)
class ZONAROJA_API UInventoryDragDropOperation : public UDragDropOperation
{
	GENERATED_BODY()

public:

	// ============================================================
	// DATOS DEL ITEM ARRASTRADO
	// ============================================================

	/** Datos del item que se está arrastrando */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Arrastre")
	FItemData DraggedItem;

	/**
	 * Índice del slot raíz en la cuadrícula de origen.
	 * -1 si el item proviene de una ranura de equipamiento.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Arrastre")
	int32 SourceRootSlot = -1;

	/** Ranura de equipamiento de origen (válida solo si bFromEquipment es true) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Arrastre")
	EEquipmentSlot SourceEquipmentSlot = EEquipmentSlot::PrimaryWeapon;

	/** Indica si el arrastre comenzó desde una ranura de equipamiento */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Arrastre")
	bool bFromEquipment = false;

	// ============================================================
	// DIMENSIONES ORIGINALES DEL ITEM
	// ============================================================

	/** Ancho original del item en celdas (sin tener en cuenta la rotación) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Arrastre|Dimensiones")
	int32 OriginalItemWidth = 1;

	/** Alto original del item en celdas (sin tener en cuenta la rotación) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Arrastre|Dimensiones")
	int32 OriginalItemHeight = 1;

	// ============================================================
	// ESTADO DE ROTACION
	// ============================================================

	/**
	 * Si el item está actualmente rotado durante el arrastre.
	 * Se alterna al pulsar R. Al soltar el item, este valor se pasa al servidor.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Arrastre|Rotación")
	bool bIsRotated = false;

	// ============================================================
	// METODOS
	// ============================================================

	/**
	 * Devuelve el ancho efectivo considerando la rotación actual.
	 * Si está rotado, el ancho y el alto se intercambian.
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Arrastre|Dimensiones")
	int32 GetEffectiveWidth() const;

	/**
	 * Devuelve el alto efectivo considerando la rotación actual.
	 * Si está rotado, el ancho y el alto se intercambian.
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Arrastre|Dimensiones")
	int32 GetEffectiveHeight() const;

	/**
	 * Alterna el estado de rotación del item durante el arrastre.
	 * Llamado cuando el jugador pulsa R con el botón del ratón presionado.
	 */
	UFUNCTION(BlueprintCallable, Category = "Arrastre|Rotación")
	void ToggleRotation();
};
