// InventoryMainWidget.h
// Widget raíz de la pantalla de inventario completa.
// Coordina la cuadrícula, el panel de equipamiento, el tooltip y el menú contextual.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Data/ZRTypes.h"
#include "InventoryMainWidget.generated.h"

class UInventoryComponent;
class UInventoryGridWidget;
class UInventoryTooltipWidget;
class UItemContextMenuWidget;
class UEquipmentSlotWidget;
class UInventoryItemWidget;

/**
 * UInventoryMainWidget
 *
 * Widget de nivel superior que orquesta toda la pantalla de inventario.
 * Se añade al viewport desde el PlayerController o el HUD al pulsar la tecla I.
 *
 * Contiene:
 *  - UInventoryGridWidget*    GridWidget        — cuadrícula de items
 *  - UInventoryTooltipWidget* TooltipWidget     — tooltip flotante de item
 *  - UItemContextMenuWidget*  ContextMenuWidget — menú contextual de clic derecho
 *  - TArray<UEquipmentSlotWidget*> EquipmentSlotWidgets — las 9 ranuras de equipo
 *
 * Flujo de inicialización:
 *  1. El HUD/PlayerController crea el widget y llama a InitializeInventory(Inventory).
 *  2. InitializeInventory propaga la referencia a todos los sub-widgets y vincula delegados.
 *  3. NativeConstruct llama a OnInventoryOpened (Blueprint) para la animación de apertura.
 *  4. NativeDestruct desvincula todos los delegados para evitar referencias colgantes.
 */
UCLASS(Abstract, BlueprintType, Blueprintable)
class ZONAROJA_API UInventoryMainWidget : public UUserWidget
{
	GENERATED_BODY()

public:

	// ============================================================
	// SUB-WIDGETS (meta=BindWidget — el Blueprint debe tener estos nombres)
	// ============================================================

	/** Cuadrícula principal del inventario */
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UInventoryGridWidget> GridWidget;

	/** Tooltip flotante que aparece al pasar el cursor sobre un item */
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UInventoryTooltipWidget> TooltipWidget;

	/** Menú contextual de clic derecho */
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UItemContextMenuWidget> ContextMenuWidget;

	// ============================================================
	// RANURAS DE EQUIPAMIENTO
	// ============================================================

	/**
	 * Array de los 9 widgets de ranura de equipamiento (uno por EEquipmentSlot).
	 * Poblado manualmente en el Blueprint hijo o mediante InitializeInventory.
	 * El orden debe coincidir con el orden del enum EEquipmentSlot.
	 */
	UPROPERTY(BlueprintReadWrite, Category = "Inventario|Equipamiento")
	TArray<TObjectPtr<UEquipmentSlotWidget>> EquipmentSlotWidgets;

	// ============================================================
	// ESTADO
	// ============================================================

	/** Referencia al componente de inventario del jugador propietario */
	UPROPERTY(BlueprintReadWrite, Category = "Inventario|Referencias")
	TObjectPtr<UInventoryComponent> OwningInventory;

	/** Texto de peso mostrado en la UI — p. ej. "32.4 / 40.0 kg" */
	UPROPERTY(BlueprintReadOnly, Category = "Inventario|Estado")
	FText WeightDisplayText;

	/** Texto de celdas libres — p. ej. "47 celdas libres" */
	UPROPERTY(BlueprintReadOnly, Category = "Inventario|Estado")
	FText FreeSpaceText;

	// ============================================================
	// METODOS DE INICIALIZACION Y REFRESCO
	// ============================================================

	/**
	 * Inicializa todos los sub-widgets con el componente de inventario del jugador.
	 * Vincula todos los delegados necesarios.
	 * Debe llamarse inmediatamente después de crear el widget y antes de mostrarlo.
	 *
	 * @param Inventory Componente de inventario del personaje del jugador.
	 */
	UFUNCTION(BlueprintCallable, Category = "Inventario")
	void InitializeInventory(UInventoryComponent* Inventory);

	/**
	 * Actualiza el texto de peso mostrado en la UI.
	 * Formato: "32.4 / 40.0 kg"
	 */
	UFUNCTION(BlueprintCallable, Category = "Inventario|Refresco")
	void RefreshWeightDisplay();

	/**
	 * Actualiza el texto de celdas libres en la UI.
	 * Formato: "47 celdas libres"
	 */
	UFUNCTION(BlueprintCallable, Category = "Inventario|Refresco")
	void RefreshFreeSpace();

	// ============================================================
	// EVENTOS IMPLEMENTABLES EN BLUEPRINT
	// ============================================================

	/**
	 * Llamado en NativeConstruct para reproducir la animación de apertura
	 * del inventario y configurar el estado inicial de la UI.
	 */
	UFUNCTION(BlueprintImplementableEvent, Category = "Inventario|Animaciones")
	void OnInventoryOpened();

	/**
	 * Llamado al cerrar/destruir el widget para reproducir la animación
	 * de cierre y restaurar el estado del juego.
	 */
	UFUNCTION(BlueprintImplementableEvent, Category = "Inventario|Animaciones")
	void OnInventoryClosed();

	/**
	 * Reproduce una animación de destello o aparición en el slot indicado
	 * cuando se añade un nuevo item al inventario.
	 * @param SlotIndex Índice del slot raíz donde aparece el nuevo item.
	 */
	UFUNCTION(BlueprintImplementableEvent, Category = "Inventario|Animaciones")
	void PlayItemAddedAnimation(int32 SlotIndex);

	// ============================================================
	// CICLO DE VIDA DEL WIDGET
	// ============================================================

	/** Inicialización nativa — llama a OnInventoryOpened */
	virtual void NativeConstruct() override;

	/** Destrucción nativa — desvincula todos los delegados */
	virtual void NativeDestruct() override;

	/**
	 * Captura la tecla R durante un arrastre activo para rotar el item.
	 * Busca la operación de arrastre activa y llama a ToggleRotation().
	 */
	virtual FReply NativeOnKeyDown(
		const FGeometry& InGeometry,
		const FKeyEvent& InKeyEvent) override;

protected:

	// ============================================================
	// CALLBACKS DE DELEGADOS (vinculados en InitializeInventory)
	// ============================================================

	/**
	 * Responde a OnInventoryChanged del componente.
	 * Actualiza el peso, el espacio libre y los datos de los slots de equipo.
	 */
	UFUNCTION()
	void HandleInventoryChanged();

	/**
	 * Responde a OnWeightChanged del componente.
	 * Actualiza WeightDisplayText y el widget de progreso de peso.
	 * @param NewWeightKg Peso actual en kilogramos.
	 */
	UFUNCTION()
	void HandleWeightChanged(float NewWeightKg);

	/**
	 * Responde a OnItemAdded del componente.
	 * Llama a PlayItemAddedAnimation para el slot afectado.
	 * @param AddedItem      Datos del item añadido.
	 * @param RootSlotIndex  Slot raíz donde se colocó.
	 */
	UFUNCTION()
	void HandleItemAdded(const FItemData& AddedItem, int32 RootSlotIndex);

	/**
	 * Recibe el evento de clic derecho desde un UInventoryItemWidget.
	 * Muestra el ContextMenuWidget posicionado en el cursor.
	 * @param Widget Widget del item sobre el que se hizo clic.
	 * @param Item   Datos del item.
	 */
	UFUNCTION()
	void HandleItemRightClicked(UInventoryItemWidget* Widget, const FItemData& Item);

	/**
	 * Recibe el evento de inicio de hover desde un UInventoryItemWidget.
	 * Rellena y muestra el TooltipWidget en la posición del cursor.
	 * @param Widget Widget del item bajo el cursor.
	 */
	UFUNCTION()
	void HandleItemHoverStart(UInventoryItemWidget* Widget);

	/**
	 * Recibe el evento de fin de hover desde un UInventoryItemWidget.
	 * Oculta el TooltipWidget.
	 * @param Widget Widget del item que el cursor acaba de abandonar.
	 */
	UFUNCTION()
	void HandleItemHoverEnd(UInventoryItemWidget* Widget);

	/**
	 * Responde a OnEquipmentChanged del componente.
	 * Refresca el widget de la ranura de equipamiento afectada.
	 * @param ChangedSlot Ranura que cambió.
	 * @param NewItem     Nuevo item equipado (o inválido si se desequipó).
	 */
	UFUNCTION()
	void HandleEquipmentChanged(EEquipmentSlot ChangedSlot, const FItemData& NewItem);

	// ============================================================
	// UTILIDADES INTERNAS
	// ============================================================

	/** Vincula los delegados de un widget de item recién creado */
	void BindItemWidgetEvents(UInventoryItemWidget* ItemWidget);

	/** Actualiza todos los widgets de ranura de equipamiento desde el estado actual */
	void RefreshAllEquipmentSlots();

	/**
	 * Devuelve la posición actual del cursor del ratón en coordenadas de pantalla.
	 * Usado para posicionar el tooltip y el menú contextual.
	 */
	FVector2D GetMouseScreenPosition() const;
};
