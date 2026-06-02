// InventoryComponent.h
// Componente de inventario tipo cuadrícula con gestión de espacio
// Soporta items de múltiples celdas, equipamiento y apilamiento

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Data/ZRTypes.h"
#include "InventoryComponent.generated.h"

// ---------------------------------------------------------
// DELEGADOS
// ---------------------------------------------------------

/** Delegado cuando se añade un item al inventario */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnItemAdded,
	const FItemData&, AddedItem, int32, SlotIndex);

/** Delegado cuando se elimina un item del inventario */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnItemRemoved,
	const FGuid&, RemovedItemID);

/** Delegado cuando cambia el equipamiento */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnEquipmentChanged,
	EEquipmentSlot, ChangedSlot, const FItemData&, NewItem);

/** Delegado cuando el inventario cambia (para actualizar la UI) */
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnInventoryChanged);

/**
 * UInventoryComponent
 * Gestiona el inventario tipo cuadrícula del jugador.
 * Similar al sistema de Escape from Tarkov: items de distinto tamaño en una rejilla.
 */
UCLASS(ClassGroup = (ZonaRoja), meta = (BlueprintSpawnableComponent))
class ZONAROJA_API UInventoryComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UInventoryComponent();

	// ---------------------------------------------------------
	// CONFIGURACION
	// ---------------------------------------------------------

	/** Número de columnas de la cuadrícula de inventario principal */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Inventario|Configuración")
	int32 GridColumns;

	/** Número de filas de la cuadrícula de inventario principal */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Inventario|Configuración")
	int32 GridRows;

	// ---------------------------------------------------------
	// ESTADO (replicado)
	// ---------------------------------------------------------

	/** Cuadrícula de ranuras del inventario */
	UPROPERTY(ReplicatedUsing = OnRep_Inventory, BlueprintReadOnly, Category = "Inventario|Estado")
	TArray<FInventorySlot> InventoryGrid;

	/** Ranuras de equipamiento del personaje */
	UPROPERTY(ReplicatedUsing = OnRep_Equipment, BlueprintReadOnly, Category = "Inventario|Estado")
	FEquipmentSlots EquipmentSlots;

	// ---------------------------------------------------------
	// FUNCIONES PRINCIPALES
	// ---------------------------------------------------------

	/**
	 * Intenta añadir un item al inventario encontrando espacio libre.
	 * @param ItemData Datos del item a añadir
	 * @param ItemWidth Ancho del item en celdas (predeterminado 1)
	 * @param ItemHeight Alto del item en celdas (predeterminado 1)
	 * @return true si el item fue añadido con éxito
	 */
	UFUNCTION(BlueprintCallable, Category = "Inventario")
	bool TryAddItem(const FItemData& ItemData, int32 ItemWidth = 1, int32 ItemHeight = 1);

	/**
	 * Añade un item en una ranura específica de la cuadrícula.
	 * @param ItemData Datos del item
	 * @param SlotIndex Índice de la ranura destino
	 * @param ItemWidth Ancho del item
	 * @param ItemHeight Alto del item
	 * @return true si el item fue colocado con éxito
	 */
	UFUNCTION(BlueprintCallable, Category = "Inventario")
	bool AddItemToSlot(const FItemData& ItemData, int32 SlotIndex, int32 ItemWidth = 1, int32 ItemHeight = 1);

	/**
	 * Elimina un item del inventario por su ID único.
	 * @param ItemID ID del item a eliminar
	 * @return true si el item fue encontrado y eliminado
	 */
	UFUNCTION(BlueprintCallable, Category = "Inventario")
	bool RemoveItem(const FGuid& ItemID);

	/**
	 * Elimina una cantidad específica de un item apilable.
	 * @param ItemDefinitionID ID de la definición del item
	 * @param Amount Cantidad a eliminar
	 * @return Cantidad realmente eliminada
	 */
	UFUNCTION(BlueprintCallable, Category = "Inventario")
	int32 RemoveItemByDefinition(const FName& ItemDefinitionID, int32 Amount);

	/**
	 * Equipa un item de la cuadrícula en la ranura de equipamiento correspondiente.
	 * @param ItemID ID del item en el inventario a equipar
	 * @param Slot Ranura de equipamiento objetivo
	 * @return true si el item fue equipado con éxito
	 */
	UFUNCTION(BlueprintCallable, Category = "Inventario|Equipamiento")
	bool EquipItem(const FGuid& ItemID, EEquipmentSlot Slot);

	/**
	 * Desequipa un item de una ranura y lo devuelve al inventario.
	 * @param Slot Ranura a desequipar
	 * @return true si el item fue desequipado y colocado en el inventario
	 */
	UFUNCTION(BlueprintCallable, Category = "Inventario|Equipamiento")
	bool UnequipItem(EEquipmentSlot Slot);

	/**
	 * Mueve un item entre dos ranuras de la cuadrícula.
	 * @param FromSlot Ranura de origen
	 * @param ToSlot Ranura de destino
	 * @return true si el movimiento fue exitoso
	 */
	UFUNCTION(BlueprintCallable, Category = "Inventario")
	bool MoveItem(int32 FromSlot, int32 ToSlot);

	// ---------------------------------------------------------
	// CONSULTAS
	// ---------------------------------------------------------

	/**
	 * Busca un item en el inventario por su ID único.
	 * @param ItemID ID del item a buscar
	 * @param OutItemData Datos del item si se encuentra
	 * @return true si el item fue encontrado
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Inventario")
	bool FindItem(const FGuid& ItemID, FItemData& OutItemData) const;

	/**
	 * Cuenta cuántas unidades de un item específico hay en el inventario.
	 * @param ItemDefinitionID ID de la definición del item a contar
	 * @return Cantidad total en el inventario
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Inventario")
	int32 GetItemCount(const FName& ItemDefinitionID) const;

	/**
	 * Verifica si hay espacio disponible para un item de las dimensiones dadas.
	 * @param ItemWidth Ancho del item en celdas
	 * @param ItemHeight Alto del item en celdas
	 * @return true si existe al menos una posición válida
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Inventario")
	bool HasSpaceFor(int32 ItemWidth, int32 ItemHeight) const;

	/**
	 * Obtiene el item equipado en una ranura específica.
	 * @param Slot Ranura de equipamiento a consultar
	 * @param OutItemData Datos del item equipado
	 * @return true si la ranura está ocupada
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Inventario|Equipamiento")
	bool GetEquippedItem(EEquipmentSlot Slot, FItemData& OutItemData) const;

	/**
	 * Devuelve el número total de celdas ocupadas en la cuadrícula.
	 * @return Número de celdas ocupadas
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Inventario")
	int32 GetUsedSlotCount() const;

	/** Capacidad total de la cuadrícula en celdas */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Inventario")
	int32 GetTotalSlotCount() const { return GridColumns * GridRows; }

	// ---------------------------------------------------------
	// DELEGADOS
	// ---------------------------------------------------------

	UPROPERTY(BlueprintAssignable, Category = "Inventario|Delegados")
	FOnItemAdded OnItemAdded;

	UPROPERTY(BlueprintAssignable, Category = "Inventario|Delegados")
	FOnItemRemoved OnItemRemoved;

	UPROPERTY(BlueprintAssignable, Category = "Inventario|Delegados")
	FOnEquipmentChanged OnEquipmentChanged;

	UPROPERTY(BlueprintAssignable, Category = "Inventario|Delegados")
	FOnInventoryChanged OnInventoryChanged;

	// ---------------------------------------------------------
	// OVERRIDES
	// ---------------------------------------------------------
	virtual void BeginPlay() override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

protected:
	/**
	 * Busca el primer slot libre que pueda contener un item de las dimensiones dadas.
	 * @param ItemWidth Ancho del item
	 * @param ItemHeight Alto del item
	 * @param OutSlotIndex Índice del slot encontrado
	 * @return true si se encontró un slot válido
	 */
	bool FindFreeSlot(int32 ItemWidth, int32 ItemHeight, int32& OutSlotIndex) const;

	/**
	 * Verifica si un item cabe en una posición específica de la cuadrícula.
	 * @param StartSlot Índice de inicio
	 * @param ItemWidth Ancho del item
	 * @param ItemHeight Alto del item
	 * @return true si el item cabe sin solaparse
	 */
	bool CanFitItemAt(int32 StartSlot, int32 ItemWidth, int32 ItemHeight) const;

	/** Inicializa la cuadrícula de inventario con slots vacíos */
	void InitializeGrid();

	/** Callback de replicación del inventario */
	UFUNCTION()
	void OnRep_Inventory();

	/** Callback de replicación del equipamiento */
	UFUNCTION()
	void OnRep_Equipment();
};
