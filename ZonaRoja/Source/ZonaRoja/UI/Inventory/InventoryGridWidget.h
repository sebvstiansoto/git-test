// InventoryGridWidget.h
// Widget principal de la cuadrícula del inventario.
// Gestiona el canvas de items y el resaltado de celdas durante el arrastre.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Data/ZRTypes.h"
#include "InventoryGridWidget.generated.h"

class UCanvasPanel;
class UCanvasPanelSlot;
class UInventoryComponent;
class UInventoryItemWidget;
class UInventoryDragDropOperation;

/**
 * UInventoryGridWidget
 *
 * Cuadrícula bidimensional del inventario. Muestra todos los items
 * como widgets hijos en un UCanvasPanel posicionados según su slot raíz.
 * Gestiona la visualización previa de posición durante el arrastre,
 * acepta operaciones de soltar y delega los RPCs al InventoryComponent.
 */
UCLASS(Abstract, BlueprintType, Blueprintable)
class ZONAROJA_API UInventoryGridWidget : public UUserWidget
{
	GENERATED_BODY()

public:

	// ============================================================
	// CONFIGURACION
	// ============================================================

	/** Tamaño de cada celda en píxeles */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cuadrícula|Config")
	float CellSize = 50.0f;

	// ============================================================
	// REFERENCIAS
	// ============================================================

	/** Referencia al componente de inventario del jugador propietario */
	UPROPERTY(BlueprintReadWrite, Category = "Cuadrícula|Referencias")
	TObjectPtr<UInventoryComponent> OwningInventory;

	/** Panel canvas donde se crean los widgets de item */
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UCanvasPanel> GridCanvas;

	// ============================================================
	// ESTADO INTERNO
	// ============================================================

	/**
	 * Índice del slot actualmente bajo el cursor durante un arrastre.
	 * -1 si no hay ningún slot activo.
	 */
	UPROPERTY(BlueprintReadOnly, Category = "Cuadrícula|Estado")
	int32 HoverSlotIndex = -1;

	// ============================================================
	// METODOS PRINCIPALES
	// ============================================================

	/**
	 * Inicializa la cuadrícula vinculando el componente de inventario
	 * y reconstruyendo los widgets de item. Llamar desde InitializeInventory
	 * del widget padre.
	 */
	UFUNCTION(BlueprintCallable, Category = "Cuadrícula")
	void InitializeGrid(UInventoryComponent* Inventory);

	/**
	 * Limpia el canvas y recrea un UInventoryItemWidget por cada slot raíz
	 * ocupado. Lo posiciona y redimensiona según (Col*TamañoCelda, Fila*TamañoCelda)
	 * y (AnchoEfectivo*TamañoCelda, AltoEfectivo*TamañoCelda).
	 */
	UFUNCTION(BlueprintCallable, Category = "Cuadrícula")
	void RebuildItemWidgets();

	/**
	 * Convierte una posición local (en píxeles dentro del canvas) al índice
	 * de slot de la cuadrícula. Devuelve -1 si está fuera de los límites.
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Cuadrícula")
	int32 GetSlotIndexAtPosition(FVector2D LocalPosition) const;

	/**
	 * Comprueba si un item de dimensiones (EffW x EffH) cabe en SlotIndex
	 * sin solapar otros items, ignorando las celdas del item excluido.
	 *
	 * @param SlotIndex        Slot raíz de destino.
	 * @param EffW             Ancho efectivo del item a colocar.
	 * @param EffH             Alto efectivo del item a colocar.
	 * @param ExcludeRootSlot  Slot raíz del item que se está moviendo (-1 = ninguno).
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Cuadrícula")
	bool CanDropAt(int32 SlotIndex, int32 EffW, int32 EffH, int32 ExcludeRootSlot) const;

	/**
	 * Resalta las celdas que ocuparía el item arrastrado si se soltara en SlotIndex.
	 * Verde = posición válida, Rojo = posición inválida o fuera de límites.
	 */
	UFUNCTION(BlueprintCallable, Category = "Cuadrícula")
	void ShowDropPreview(int32 SlotIndex, int32 EffW, int32 EffH, bool bValid);

	// ============================================================
	// EVENTOS IMPLEMENTABLES EN BLUEPRINT
	// ============================================================

	/**
	 * Resalta o quita el resaltado de una celda individual.
	 * El Blueprint dibuja el fondo de la celda en verde o rojo.
	 */
	UFUNCTION(BlueprintImplementableEvent, Category = "Cuadrícula|Visual")
	void SetCellHighlight(int32 SlotIndex, bool bHighlight, bool bIsValid);

	/** Quita todos los resaltados de la cuadrícula. */
	UFUNCTION(BlueprintImplementableEvent, Category = "Cuadrícula|Visual")
	void ClearAllHighlights();

	// ============================================================
	// EVENTOS NATIVOS DE DRAG & DROP
	// ============================================================

	virtual bool NativeOnDragOver(
		const FGeometry& InGeometry,
		const FDragDropEvent& InDragDropEvent,
		UDragDropOperation* InOperation) override;

	virtual bool NativeOnDrop(
		const FGeometry& InGeometry,
		const FDragDropEvent& InDragDropEvent,
		UDragDropOperation* InOperation) override;

	virtual void NativeOnDragLeave(
		const FDragDropEvent& InDragDropEvent,
		UDragDropOperation* InOperation) override;

	virtual FReply NativeOnMouseMove(
		const FGeometry& InGeometry,
		const FPointerEvent& InMouseEvent) override;

	/** Clase del widget de item a instanciar. Debe establecerse en el Blueprint hijo. */
	UPROPERTY(EditDefaultsOnly, Category = "Cuadrícula|Config")
	TSubclassOf<UInventoryItemWidget> ItemWidgetClass;

	/**
	 * Lista de widgets de item actualmente instanciados en el canvas.
	 * Expuesto públicamente para que UInventoryMainWidget pueda vincular
	 * los delegados OnRightClicked, OnHoverStart y OnHoverEnd tras RebuildItemWidgets.
	 */
	UPROPERTY(BlueprintReadOnly, Category = "Cuadrícula|Estado")
	TArray<TObjectPtr<UInventoryItemWidget>> ActiveItemWidgets;

protected:

	// ============================================================
	// CALLBACKS INTERNOS
	// ============================================================

	/** Llamado cuando OnInventoryChanged se emite — reconstruye los widgets */
	UFUNCTION()
	void HandleInventoryChanged();

	/** Vincula los delegados (OnRightClicked, OnHoverStart, OnHoverEnd) de un widget de item */
	void BindItemWidgetDelegates(UInventoryItemWidget* ItemWidget);
};
