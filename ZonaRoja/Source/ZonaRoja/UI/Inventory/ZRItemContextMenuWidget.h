// ZRItemContextMenuWidget.h
// Menú contextual de clic derecho para acciones sobre items del inventario.
// Muestra opciones como equipar, tirar, dividir stack, inspeccionar y desechar.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Data/ZRTypes.h"
#include "ZRItemContextMenuWidget.generated.h"

class UInventoryComponent;

// ============================================================
// DELEGADOS
// ============================================================

/**
 * Emitido cuando el jugador solicita inspeccionar un item.
 * El sistema de inspección 3D escucha este delegado para mostrar
 * el modelo del item en una cámara dedicada.
 */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnInspectRequested,
	const FItemData&, Item);

/**
 * UItemContextMenuWidget
 *
 * Menú flotante de clic derecho que aparece al hacer RMB sobre un item
 * de la cuadrícula de inventario o sobre una ranura de equipamiento.
 *
 * Se posiciona en la posición del cursor mediante PositionAt (Blueprint).
 * Los botones visibles se activan/desactivan con SetupMenuItems según el contexto.
 *
 * Todas las acciones que mutan el inventario envían RPCs al servidor.
 */
UCLASS(Abstract, BlueprintType, Blueprintable)
class ZONAROJA_API UZRItemContextMenuWidget : public UUserWidget
{
	GENERATED_BODY()

public:

	// ============================================================
	// ESTADO
	// ============================================================

	/** Datos del item sobre el que se abrió el menú */
	UPROPERTY(BlueprintReadWrite, Category = "Menú Contextual|Estado")
	FItemData ContextItem;

	/** Índice del slot raíz del item en la cuadrícula (-1 si viene de equipo) */
	UPROPERTY(BlueprintReadWrite, Category = "Menú Contextual|Estado")
	int32 ItemRootSlot = -1;

	/** Referencia al componente de inventario del jugador propietario */
	UPROPERTY(BlueprintReadWrite, Category = "Menú Contextual|Referencias")
	TObjectPtr<UInventoryComponent> OwningInventory;

	// ============================================================
	// DELEGADOS PÚBLICOS
	// ============================================================

	/** Emitido al solicitar la inspección del item */
	UPROPERTY(BlueprintAssignable, Category = "Menú Contextual|Delegados")
	FOnInspectRequested OnInspectRequested;

	// ============================================================
	// METODOS DE CONTROL
	// ============================================================

	/**
	 * Configura y muestra el menú para el item indicado.
	 * Almacena los datos del item, activa los botones pertinentes y
	 * llama a los eventos Blueprint PositionAt y SetupMenuItems.
	 *
	 * @param Item           Datos del item sobre el que se abrió el menú.
	 * @param RootSlot       Índice del slot raíz en la cuadrícula.
	 * @param Inventory      Componente de inventario del propietario.
	 * @param ScreenPosition Posición del cursor en pantalla para posicionar el menú.
	 */
	UFUNCTION(BlueprintCallable, Category = "Menú Contextual")
	void ShowForItem(
		const FItemData& Item,
		int32 RootSlot,
		UInventoryComponent* Inventory,
		FVector2D ScreenPosition);

	/**
	 * Oculta el menú contextual y limpia los datos almacenados.
	 * Llamar al seleccionar cualquier opción o al hacer clic fuera del menú.
	 */
	UFUNCTION(BlueprintCallable, Category = "Menú Contextual")
	void Hide();

	// ============================================================
	// ACCIONES DEL MENU (botones del Blueprint llaman a estas funciones)
	// ============================================================

	/**
	 * Equipa el item en la mejor ranura disponible.
	 * Determina el slot óptimo según el tipo de item y llama a
	 * ServerRequestEquipItem en el componente de inventario.
	 *
	 * TODO: La determinación del slot óptimo requiere consultar
	 * FItemDefinitionRow::ItemType del DataTable DT_ItemDefinitions.
	 */
	UFUNCTION(BlueprintCallable, Category = "Menú Contextual|Acciones")
	void OnClickEquip();

	/**
	 * Tira el item al mundo en la posición del personaje.
	 * Llama a ServerRequestDropItem. El servidor instancia el actor de pickup.
	 */
	UFUNCTION(BlueprintCallable, Category = "Menú Contextual|Acciones")
	void OnClickDrop();

	/**
	 * Inicia la división de un stack.
	 * Solo disponible si ContextItem.StackCount > 1.
	 * Llama a ServerRequestSplitStack con la mitad del stack como cantidad inicial.
	 *
	 * TODO: Mostrar un slider/prompt para que el jugador elija la cantidad exacta.
	 * Por ahora divide el stack por la mitad redondeando hacia abajo.
	 */
	UFUNCTION(BlueprintCallable, Category = "Menú Contextual|Acciones")
	void OnClickSplitStack();

	/**
	 * Solicita la inspección 3D del item.
	 * Emite OnInspectRequested con los datos del item para que el
	 * sistema de inspección lo procese.
	 */
	UFUNCTION(BlueprintCallable, Category = "Menú Contextual|Acciones")
	void OnClickInspect();

	/**
	 * Descarta permanentemente el item del inventario.
	 * En un proyecto final mostraría un popup de confirmación.
	 * Actualmente llama directamente a ServerRequestDropItem seguido de
	 * una destrucción del pickup en el servidor.
	 *
	 * TODO: Mostrar popup de confirmación antes de eliminar el item.
	 */
	UFUNCTION(BlueprintCallable, Category = "Menú Contextual|Acciones")
	void OnClickDiscard();

	// ============================================================
	// EVENTOS IMPLEMENTABLES EN BLUEPRINT
	// ============================================================

	/**
	 * Configura la visibilidad de cada botón según el contexto del item.
	 * @param bCanEquip    Si el item puede equiparse en alguna ranura.
	 * @param bCanSplit    Si el stack tiene más de 1 unidad.
	 * @param bCanDiscard  Si el item puede descartarse (no es item de misión).
	 */
	UFUNCTION(BlueprintImplementableEvent, Category = "Menú Contextual|Visual")
	void SetupMenuItems(bool bCanEquip, bool bCanSplit, bool bCanDiscard);

	/**
	 * Posiciona el menú en la posición indicada de la pantalla.
	 * El Blueprint ajusta los márgenes para que el menú no salga de la pantalla.
	 * @param ScreenPosition Posición del cursor en coordenadas de pantalla.
	 */
	UFUNCTION(BlueprintImplementableEvent, Category = "Menú Contextual|Visual")
	void PositionAt(FVector2D ScreenPosition);

protected:

	/**
	 * Determina si el item puede equiparse en alguna ranura de equipamiento.
	 * TODO: Consultar FItemDefinitionRow::ItemType para determinar compatibilidad real.
	 */
	bool CanItemBeEquipped() const;
};
