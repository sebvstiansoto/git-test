// ZRAITypes.h
// Tipos compartidos para el sistema de inteligencia artificial de ZonaRoja
// Define enumerados y estructuras usados por los PMCs (contratistas militares privados)

#pragma once

#include "CoreMinimal.h"
#include "ZRAITypes.generated.h"

// ============================================================
// ENUMERADOS DE ESTADO Y COMPORTAMIENTO DE PMC
// ============================================================

/**
 * Estados principales de la máquina de estados del PMC.
 * La transición sigue el flujo: Patrol → Suspicious → Alert → Combat → Dead
 */
UENUM(BlueprintType)
enum class EPMCState : uint8
{
	Patrol		UMETA(DisplayName = "Patrulla"),		// Siguiendo puntos de patrulla con pausas aleatorias
	Suspicious	UMETA(DisplayName = "Sospechoso"),		// Escuchó algo, se mueve a investigar
	Alert		UMETA(DisplayName = "Alerta"),			// Detectó al jugador, busca su última posición conocida
	Combat		UMETA(DisplayName = "Combate"),			// Enfrentamiento activo usando cobertura
	Dead		UMETA(DisplayName = "Muerto")			// Ragdoll, ya no está activo
};

/**
 * Nivel de competencia del PMC.
 * Determina sus estadísticas de combate: puntería, velocidad de reacción, comportamiento táctico.
 */
UENUM(BlueprintType)
enum class EPMCTier : uint8
{
	Novice		UMETA(DisplayName = "Novato"),			// Poco entrenado, baja precisión y tiempo de reacción lento
	Veteran		UMETA(DisplayName = "Veterano"),		// Entrenamiento estándar, puede llamar refuerzos
	Elite		UMETA(DisplayName = "Élite")			// Altamente entrenado, puede flanquear, máxima precisión
};

/**
 * Acciones tácticas disponibles mientras el PMC está en cobertura.
 * Determina la animación y el comportamiento de combate desde posición cubierta.
 */
UENUM(BlueprintType)
enum class ECoverAction : uint8
{
	None			UMETA(DisplayName = "Ninguna"),			// Sin acción de cobertura activa
	TakingCover		UMETA(DisplayName = "Tomando Cobertura"),	// Moviéndose hacia la cobertura
	PeekingLeft		UMETA(DisplayName = "Asomando Izquierda"),	// Asomándose por el lado izquierdo
	PeekingRight	UMETA(DisplayName = "Asomando Derecha"),	// Asomándose por el lado derecho
	StandingUp		UMETA(DisplayName = "Levantándose")			// Levantándose de la cobertura para atacar
};

// ============================================================
// ESTRUCTURAS DE CONFIGURACION
// ============================================================

/**
 * Configuración de estadísticas de combate por nivel de PMC.
 * Se asigna automáticamente en BeginPlay según el EPMCTier del personaje.
 */
USTRUCT(BlueprintType)
struct ZONAROJA_API FPMCStatConfig
{
	GENERATED_BODY()

	// ---- Percepción ----

	/** Radio máximo de visión en centímetros (Novato=2000, Veterano=3000, Élite=4000) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Percepción")
	float SightRadius = 2000.0f;

	/** Rango de audición en centímetros (Novato=1500, Veterano=2000, Élite=2500) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Percepción")
	float HearingRange = 1500.0f;

	// ---- Combate ----

	/** Tiempo en segundos antes de abrir fuego tras detectar al jugador (Novato=0.8, Veterano=0.4, Élite=0.2) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combate")
	float ReactionTime = 0.8f;

	/** Dispersión de los disparos en grados (Novato=4.0, Veterano=2.0, Élite=0.8) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combate")
	float AccuracySpread = 4.0f;

	/** Tiempo en segundos entre disparos consecutivos (Novato=0.4, Veterano=0.25, Élite=0.15) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combate")
	float FireRate = 0.4f;

	// ---- Velocidades de movimiento (cm/s) ----

	/** Velocidad durante la patrulla normal */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movimiento")
	float PatrolSpeed = 200.0f;

	/** Velocidad cuando está en estado de alerta */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movimiento")
	float AlertSpeed = 350.0f;

	/** Velocidad durante el combate activo */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movimiento")
	float CombatSpeed = 400.0f;

	// ---- Comportamiento táctico ----

	/** Radio en centímetros para llamar refuerzos cercanos */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Táctica")
	int32 MaxGroupCallRadius = 3000;

	/**
	 * Si este PMC intentará flanquear al jugador (solo Élite).
	 * Cuando es true, el PMC busca posiciones alternativas para atacar desde ángulos inesperados.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Táctica")
	bool bCanFlank = false;

	/**
	 * Si este PMC alerta a los PMCs cercanos al detectar al jugador (Veterano y Élite).
	 * Activa SetState(Alert) en todos los PMCs dentro de MaxGroupCallRadius.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Táctica")
	bool bCallsForBackup = false;

	FPMCStatConfig() {}
};

/**
 * Punto de patrulla en la ruta asignada al PMC.
 * Los PMCs siguen los puntos en orden secuencial, con comportamiento configurable en cada uno.
 */
USTRUCT(BlueprintType)
struct ZONAROJA_API FPatrolPoint
{
	GENERATED_BODY()

	/** Posición en el mundo de este punto de patrulla */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Patrulla")
	FVector Location = FVector::ZeroVector;

	/** Tiempo en segundos que el PMC espera al llegar a este punto */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Patrulla")
	float WaitTime = 3.0f;

	/**
	 * Si el PMC debe rotar aleatoriamente mientras espera en este punto.
	 * Simula vigilancia activa del entorno.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Patrulla")
	bool bLookAround = true;

	FPatrolPoint() {}

	FPatrolPoint(FVector InLocation, float InWaitTime = 3.0f, bool bInLookAround = true)
		: Location(InLocation)
		, WaitTime(InWaitTime)
		, bLookAround(bInLookAround)
	{}
};
