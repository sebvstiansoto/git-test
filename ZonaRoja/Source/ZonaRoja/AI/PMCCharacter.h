// PMCCharacter.h
// Personaje enemigo PMC (Contratista Militar Privado) para ZonaRoja
// Gestiona percepción, estados de IA, cobertura y replicación de red

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "Perception/AIPerceptionComponent.h"
#include "AI/ZRAITypes.h"
#include "Data/ZRTypes.h"
#include "Net/UnrealNetwork.h"
#include "PMCCharacter.generated.h"

// Declaraciones anticipadas
class UHealthComponent;
class UAIPerceptionComponent;
class APMCAIController;
class AZRCoverPoint;
class AZRPlayerCharacter;

/**
 * APMCCharacter
 * Personaje enemigo de tipo PMC con sistema de estados, percepción y combate.
 * Utiliza un AIController (APMCAIController) y un BehaviorTree para su lógica.
 * Soporta replicación de estado actual y última posición conocida del jugador.
 */
UCLASS(Blueprintable, BlueprintType)
class ZONAROJA_API APMCCharacter : public ACharacter
{
	GENERATED_BODY()

public:
	APMCCharacter();

	// ============================================================
	// CONFIGURACION DEL PMC
	// ============================================================

	/** Nivel de competencia del PMC — determina todas las estadísticas de combate */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "PMC|Configuración")
	EPMCTier Tier = EPMCTier::Novice;

	/** Lista de puntos de patrulla que seguirá el PMC en orden cíclico */
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "PMC|Patrulla")
	TArray<FPatrolPoint> PatrolPoints;

	/**
	 * Configuración de estadísticas calculada automáticamente en BeginPlay
	 * según el EPMCTier asignado. También se puede sobreescribir manualmente.
	 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "PMC|Estadísticas")
	FPMCStatConfig StatConfig;

	// ============================================================
	// COMPONENTES
	// ============================================================

	/** Componente de salud con sistema de daño por partes del cuerpo */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Componentes")
	TObjectPtr<UHealthComponent> HealthComp;

	/** Componente de percepción con sensores de visión y audición */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Componentes")
	TObjectPtr<UAIPerceptionComponent> PerceptionComp;

	/** Mesh del arma del PMC para obtener la posición del cañón */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Componentes")
	TObjectPtr<USkeletalMeshComponent> WeaponMesh;

	// ============================================================
	// ESTADO REPLICADO
	// ============================================================

	/** Estado actual de la máquina de estados del PMC (replicado) */
	UPROPERTY(ReplicatedUsing = OnRep_CurrentState, BlueprintReadOnly, Category = "PMC|Estado")
	EPMCState CurrentState = EPMCState::Patrol;

	/** Última posición conocida del jugador, usada para búsqueda y alerta (replicada) */
	UPROPERTY(ReplicatedUsing = OnRep_LastKnownLocation, BlueprintReadOnly, Category = "PMC|Estado")
	FVector LastKnownPlayerLocation = FVector::ZeroVector;

	// ============================================================
	// ESTADO INTERNO (no replicado)
	// ============================================================

	/** Índice del punto de patrulla actual en el array PatrolPoints */
	UPROPERTY(BlueprintReadOnly, Category = "PMC|Patrulla")
	int32 CurrentPatrolIndex = 0;

	/** Tiempo acumulado desde el último disparo para controlar la cadencia de fuego */
	UPROPERTY(BlueprintReadOnly, Category = "PMC|Combate")
	float TimeSinceLastShot = 0.0f;

	/** Acción táctica de cobertura que está ejecutando actualmente */
	UPROPERTY(BlueprintReadOnly, Category = "PMC|Cobertura")
	ECoverAction CurrentCoverAction = ECoverAction::None;

	/** Actor de cobertura que está usando actualmente el PMC */
	UPROPERTY(BlueprintReadOnly, Category = "PMC|Cobertura")
	TObjectPtr<AActor> CurrentCoverActor;

	// ============================================================
	// FUNCIONES PRINCIPALES
	// ============================================================

	/**
	 * Cambia el estado de la máquina de estados del PMC.
	 * Notifica al AIController para sincronizar el Blackboard.
	 * @param NewState Nuevo estado al que transicionar
	 */
	UFUNCTION(BlueprintCallable, Category = "PMC|Estado")
	void SetState(EPMCState NewState);

	/**
	 * Callback del sistema de percepción cuando detecta un estímulo.
	 * Maneja tanto visión (AIStimulus.Type=Sight) como audición (Hearing).
	 * @param Actor Actor que generó el estímulo
	 * @param Stimulus Datos del estímulo percibido
	 */
	UFUNCTION()
	void OnPerceptionUpdated(AActor* Actor, FAIStimulus Stimulus);

	/**
	 * Responde al daño recibido.
	 * Si está en combate: orienta hacia el origen del daño.
	 * Si no está en combate: transiciona a estado de alerta.
	 * @param Damage Cantidad de daño recibido
	 * @param Part Parte del cuerpo impactada
	 * @param Type Tipo de daño
	 * @param DamageInstigator Actor que causó el daño
	 * @param HitResult Datos del impacto
	 */
	UFUNCTION()
	void OnTakeDamage(float Damage, EBodyPart Part, EDamageType Type,
		AActor* DamageInstigator, const FHitResult& HitResult);

	/**
	 * Llama refuerzos mediante sphere overlap.
	 * Activa SetState(Alert) en todos los PMCs dentro de MaxGroupCallRadius.
	 */
	UFUNCTION(BlueprintCallable, Category = "PMC|Táctica")
	void CallForBackup();

	/**
	 * Obtiene la posición del cañón desde el socket "Muzzle" del WeaponMesh.
	 * @return Posición mundial del cañón del arma
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "PMC|Combate")
	FVector GetMuzzleLocation() const;

	/**
	 * Verifica si hay línea de visión directa al jugador.
	 * Realiza un trace que ignora a los compañeros PMCs.
	 * @return true si puede ver al jugador directamente
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "PMC|Percepción")
	bool HasLineOfSightToPlayer() const;

	/**
	 * Aplica la configuración de estadísticas según el Tier asignado.
	 * Se llama en BeginPlay. Novato/Veterano/Élite tienen valores predefinidos.
	 */
	UFUNCTION(BlueprintCallable, Category = "PMC|Configuración")
	void ApplyTierConfig();

	/**
	 * Obtiene el jugador más cercano en la escena.
	 * @return Puntero al AZRPlayerCharacter más cercano, o nullptr si no hay ninguno
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "PMC|Percepción")
	AZRPlayerCharacter* GetNearestPlayer() const;

	// ============================================================
	// OVERRIDES DE ACHARACTER
	// ============================================================

	virtual void BeginPlay() override;
	virtual void Tick(float DeltaTime) override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

protected:
	// ============================================================
	// CALLBACKS DE MUERTE Y REPLICACION
	// ============================================================

	/**
	 * Se llama cuando el HealthComp notifica la muerte del PMC.
	 * Activa ragdoll, desactiva colisión e IA, y genera contenedor de botín.
	 * @param Victim Actor que murió (este mismo PMC)
	 * @param Killer Actor que causó la muerte
	 */
	UFUNCTION()
	void OnPMCDeath(AActor* Victim, AActor* Killer);

	/**
	 * Callback de replicación del estado actual.
	 * Actualiza el Animation Blueprint cuando el estado cambia en clientes.
	 */
	UFUNCTION()
	void OnRep_CurrentState();

	/**
	 * Callback de replicación de la última posición conocida del jugador.
	 * Utilizado para efectos visuales de búsqueda en clientes.
	 */
	UFUNCTION()
	void OnRep_LastKnownLocation();

	/**
	 * Actualiza el Animation Blueprint con el estado actual.
	 * Se llama en el servidor tras SetState y en clientes tras OnRep_CurrentState.
	 */
	void UpdateAnimBP();

	/** Referencia cacheada al AIController para acceso rápido */
	UPROPERTY()
	TObjectPtr<APMCAIController> CachedAIController;

	/** Tiempo acumulado sin línea de visión, usado para transición de combate a alerta */
	float TimeSinceLastSight = 0.0f;

	/** Si el PMC está en proceso de muerte (evita múltiples llamadas) */
	bool bIsDeathProcessed = false;
};
