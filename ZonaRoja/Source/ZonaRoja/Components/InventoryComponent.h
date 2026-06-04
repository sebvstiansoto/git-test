// InventoryComponent.h
// Inventario tipo cuadrícula con peso, rotación, split de stacks,
// sub-inventarios de contenedor y RPCs servidor-autoridad.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Data/ZRTypes.h"
#include "InventoryComponent.generated.h"

// ============================================================
// RESULTADO DE OPERACION DE INVENTARIO
// ============================================================

UENUM(BlueprintType)
enum class EInventoryResult : uint8
{
	Success				UMETA(DisplayName = "Éxito"),
	NoSpace				UMETA(DisplayName = "Sin Espacio"),
	ItemNotFound		UMETA(DisplayName = "Item No Encontrado"),
	InvalidSlot			UMETA(DisplayName = "Ranura Inválida"),
	SlotOccupied		UMETA(DisplayName = "Ranura Ocupada"),
	WeightExceeded		UMETA(DisplayName = "Peso Excedido"),
	NotAuthority		UMETA(DisplayName = "Sin Autoridad"),
	InvalidItem			UMETA(DisplayName = "Item Inválido"),
	StackFull			UMETA(DisplayName = "Stack Lleno"),
	CannotStack			UMETA(DisplayName = "No Apilable"),
};

// ============================================================
// DELEGADOS
// ============================================================

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnItemAdded,
	const FItemData&, AddedItem, int32, RootSlotIndex);

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnItemRemoved,
	FGuid, RemovedItemID);

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnItemMoved,
	FGuid, ItemID, int32, NewRootSlotIndex);

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnEquipmentChanged,
	EEquipmentSlot, ChangedSlot, const FItemData&, NewItem);

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnInventoryChanged);

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnWeightChanged,
	float, NewWeightKg);

// ============================================================
// COMPONENTE DE INVENTARIO
// ============================================================

/**
 * UInventoryComponent
 *
 * Inventario de cuadrícula bidimensional para extracción shooter.
 * - Items de múltiples celdas con rotación opcional
 * - Sistema de peso con penalizaciones de movimiento
 * - Sub-inventarios dinámicos (mochila, chaleco)
 * - Todas las mutaciones son server-authoritative
 * - RPCs para operaciones iniciadas por el cliente
 */
UCLASS(ClassGroup = (ZonaRoja), meta = (BlueprintSpawnableComponent))
class ZONAROJA_API UInventoryComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UInventoryComponent();

	// ============================================================
	// CONFIGURACION (servidor y cliente)
	// ============================================================

	/** Columnas de la cuadrícula base (sin contenedores equipados) */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Inventario|Config")
	int32 BaseGridColumns = 5;

	/** Filas de la cuadrícula base */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Inventario|Config")
	int32 BaseGridRows = 3;

	/** Peso máximo base en gramos (sin modificadores de equipo) */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Inventario|Config")
	int32 BaseMaxWeightGrams = 40000;

	// ============================================================
	// ESTADO REPLICADO
	// ============================================================

	/** Cuadrícula de slots del inventario (incluye slots de contenedores equipados) */
	UPROPERTY(ReplicatedUsing = OnRep_Grid, BlueprintReadOnly, Category = "Inventario|Estado")
	TArray<FInventorySlot> Grid;

	/** Ranuras de equipamiento del cuerpo */
	UPROPERTY(ReplicatedUsing = OnRep_Equipment, BlueprintReadOnly, Category = "Inventario|Estado")
	FEquipmentSlots Equipment;

	/** Peso total actual del inventario en gramos */
	UPROPERTY(ReplicatedUsing = OnRep_Weight, BlueprintReadOnly, Category = "Inventario|Estado")
	int32 CurrentWeightGrams = 0;

	/** Dimensiones actuales de la cuadrícula (pueden cambiar al equipar mochila/chaleco) */
	UPROPERTY(Replicated, BlueprintReadOnly, Category = "Inventario|Estado")
	int32 GridColumns = 5;

	UPROPERTY(Replicated, BlueprintReadOnly, Category = "Inventario|Estado")
	int32 GridRows = 3;

	// ============================================================
	// OPERACIONES PRINCIPALES (solo servidor)
	// ============================================================

	/**
	 * Intenta añadir un item al primer slot libre encontrando automáticamente
	 * su posición. Si el item es apilable y ya existe en el inventario,
	 * primero intenta sumar al stack existente.
	 */
	UFUNCTION(BlueprintCallable, Category = "Inventario")
	EInventoryResult TryAddItem(const FItemData& ItemData, int32 ItemWidth = 1, int32 ItemHeight = 1);

	/**
	 * Coloca un item en una posición específica de la cuadrícula.
	 * @param bRotated Si se coloca rotado 90° (intercambia ancho y alto)
	 */
	UFUNCTION(BlueprintCallable, Category = "Inventario")
	EInventoryResult PlaceItemAt(const FItemData& ItemData, int32 SlotIndex,
		int32 ItemWidth = 1, int32 ItemHeight = 1, bool bRotated = false);

	/** Elimina un item del inventario por su ID de instancia */
	UFUNCTION(BlueprintCallable, Category = "Inventario")
	EInventoryResult RemoveItem(const FGuid& ItemID);

	/**
	 * Mueve un item de un slot raíz a otro.
	 * Si el destino está ocupado, intenta intercambiar los items.
	 * @param bRotateOnMove Si se rota el item al moverlo
	 */
	UFUNCTION(BlueprintCallable, Category = "Inventario")
	EInventoryResult MoveItem(int32 FromRootSlot, int32 ToRootSlot, bool bRotateOnMove = false);

	/**
	 * Divide un stack en dos. Deja (Amount) en el slot original
	 * y crea un nuevo item con el resto en el primer slot libre.
	 * @param ItemID ID del stack a dividir
	 * @param Amount Cantidad que queda en el slot original
	 */
	UFUNCTION(BlueprintCallable, Category = "Inventario")
	EInventoryResult SplitStack(const FGuid& ItemID, int32 Amount);

	/**
	 * Elimina una cantidad de un item apilable por su definición.
	 * Recorre stacks de menor a mayor tamaño para minimizar fragmentación.
	 * @return Cantidad realmente eliminada
	 */
	UFUNCTION(BlueprintCallable, Category = "Inventario")
	int32 RemoveAmountByDefinition(const FName& ItemDefinitionID, int32 Amount);

	/**
	 * Equipa un item del inventario en la ranura de equipamiento indicada.
	 * Si la ranura ya está ocupada, intercambia con el item actual.
	 */
	UFUNCTION(BlueprintCallable, Category = "Inventario|Equipamiento")
	EInventoryResult EquipItem(const FGuid& ItemID, EEquipmentSlot Slot);

	/** Desequipa la ranura y devuelve el item al inventario */
	UFUNCTION(BlueprintCallable, Category = "Inventario|Equipamiento")
	EInventoryResult UnequipItem(EEquipmentSlot Slot);

	/**
	 * Ordena automáticamente el inventario (compacta items hacia arriba-izquierda).
	 * No reordena por tipo, solo compacta el espacio libre.
	 */
	UFUNCTION(BlueprintCallable, Category = "Inventario")
	void AutoSort();

	// ============================================================
	// RPCS: CLIENTE → SERVIDOR
	// El cliente pide la operación; el servidor valida y ejecuta.
	// ============================================================

	UFUNCTION(Server, Reliable, BlueprintCallable, Category = "Inventario|RPC")
	void ServerRequestMoveItem(int32 FromRootSlot, int32 ToRootSlot, bool bRotate);

	UFUNCTION(Server, Reliable, BlueprintCallable, Category = "Inventario|RPC")
	void ServerRequestDropItem(FGuid ItemID);

	UFUNCTION(Server, Reliable, BlueprintCallable, Category = "Inventario|RPC")
	void ServerRequestSplitStack(FGuid ItemID, int32 Amount);

	UFUNCTION(Server, Reliable, BlueprintCallable, Category = "Inventario|RPC")
	void ServerRequestEquipItem(FGuid ItemID, EEquipmentSlot Slot);

	UFUNCTION(Server, Reliable, BlueprintCallable, Category = "Inventario|RPC")
	void ServerRequestUnequipItem(EEquipmentSlot Slot);

	UFUNCTION(Server, Reliable, BlueprintCallable, Category = "Inventario|RPC")
	void ServerRequestAutoSort();

	// ============================================================
	// CONSULTAS (const, disponibles en cliente y servidor)
	// ============================================================

	/** Busca un item por ID. Solo busca en slots raíz. */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Inventario|Consulta")
	bool FindItem(const FGuid& ItemID, FItemData& OutItemData) const;

	/** Devuelve el índice del slot raíz de un item (-1 si no existe) */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Inventario|Consulta")
	int32 FindItemRootSlot(const FGuid& ItemID) const;

	/** Cuenta unidades totales de un tipo de item (suma stacks) */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Inventario|Consulta")
	int32 GetItemCount(const FName& ItemDefinitionID) const;

	/** Devuelve true si hay espacio para un item de las dimensiones indicadas */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Inventario|Consulta")
	bool HasSpaceFor(int32 ItemWidth, int32 ItemHeight) const;

	/** Devuelve el item equipado en una ranura (false si está vacía) */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Inventario|Consulta")
	bool GetEquippedItem(EEquipmentSlot Slot, FItemData& OutItemData) const;

	/** Peso actual en kilogramos (para la UI) */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Inventario|Consulta")
	float GetCurrentWeightKg() const { return CurrentWeightGrams / 1000.0f; }

	/** Peso máximo en kilogramos */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Inventario|Consulta")
	float GetMaxWeightKg() const { return GetMaxWeightGrams() / 1000.0f; }

	/** Ratio peso/máximo (0.0 – 1.0+). >1.0 = sobrecargado */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Inventario|Consulta")
	float GetWeightRatio() const;

	/** Número de celdas libres en la cuadrícula */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Inventario|Consulta")
	int32 GetFreeSlotCount() const;

	/** Número de celdas totales (GridColumns * GridRows) */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Inventario|Consulta")
	int32 GetTotalSlotCount() const { return GridColumns * GridRows; }

	/**
	 * Devuelve todos los items únicos del inventario (sin duplicar por celdas secundarias).
	 * Útil para serialización y UI.
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Inventario|Consulta")
	TArray<FItemData> GetAllItems() const;

	/** Calcula el valor total de loot en CZ (requiere acceso a DataTable) */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Inventario|Consulta")
	int32 GetTotalLootValueCZ() const;

	// ============================================================
	// DELEGADOS
	// ============================================================

	UPROPERTY(BlueprintAssignable, Category = "Inventario|Delegados")
	FOnItemAdded OnItemAdded;

	UPROPERTY(BlueprintAssignable, Category = "Inventario|Delegados")
	FOnItemRemoved OnItemRemoved;

	UPROPERTY(BlueprintAssignable, Category = "Inventario|Delegados")
	FOnItemMoved OnItemMoved;

	UPROPERTY(BlueprintAssignable, Category = "Inventario|Delegados")
	FOnEquipmentChanged OnEquipmentChanged;

	UPROPERTY(BlueprintAssignable, Category = "Inventario|Delegados")
	FOnInventoryChanged OnInventoryChanged;

	UPROPERTY(BlueprintAssignable, Category = "Inventario|Delegados")
	FOnWeightChanged OnWeightChanged;

	// ============================================================
	// CICLO DE VIDA
	// ============================================================

	virtual void BeginPlay() override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

protected:
	// ============================================================
	// UTILIDADES INTERNAS
	// ============================================================

	/** Reconstruye la cuadrícula con las dimensiones actuales. Llama tras equipar mochila/chaleco. */
	void RebuildGrid();

	/**
	 * Calcula las dimensiones totales de la cuadrícula según el equipo.
	 * La cuadrícula es siempre un único rectángulo; el chaleco y la mochila
	 * añaden filas/columnas al bloque base.
	 */
	void RecalculateGridSize();

	/** Recalcula CurrentWeightGrams a partir de todos los items del grid y equipamiento */
	void RecalculateWeight();

	/** Peso máximo en gramos (base + modificadores de equipo) */
	int32 GetMaxWeightGrams() const;

	/** Busca la primera posición libre para un item. Devuelve -1 si no hay espacio. */
	int32 FindFirstFreeSlot(int32 ItemWidth, int32 ItemHeight, bool bTryRotated = false) const;

	/** Verifica si un item de (W x H) cabe en SlotIndex sin solapar otros items. */
	bool CanFitAt(int32 SlotIndex, int32 EffectiveWidth, int32 EffectiveHeight) const;

	/** Marca/desmarca las celdas que ocupa un item. */
	void OccupyCells(int32 RootSlot, const FItemData& ItemData,
		int32 EffectiveWidth, int32 EffectiveHeight, bool bRotated, bool bOccupy);

	/** Devuelve el puntero al FItemData del slot de equipamiento (para escritura). */
	FItemData* GetEquipmentSlotPtr(EEquipmentSlot Slot);
	const FItemData* GetEquipmentSlotPtr(EEquipmentSlot Slot) const;

	// ============================================================
	// CALLBACKS DE REPLICACIÓN
	// ============================================================

	UFUNCTION()
	void OnRep_Grid();

	UFUNCTION()
	void OnRep_Equipment();

	UFUNCTION()
	void OnRep_Weight();
};
