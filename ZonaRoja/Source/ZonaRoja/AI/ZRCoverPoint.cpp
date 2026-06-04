// ZRCoverPoint.cpp
// Implementación del actor de punto de cobertura
// Gestiona ocupación y validación de línea de visión

#include "AI/ZRCoverPoint.h"
#include "AI/PMCCharacter.h"

#include "Components/SphereComponent.h"
#include "DrawDebugHelpers.h"

AZRCoverPoint::AZRCoverPoint()
{
	PrimaryActorTick.bCanEverTick = false;

	// Crear el volumen esférico de overlap para consultas de cobertura
	OverlapVolume = CreateDefaultSubobject<USphereComponent>(TEXT("OverlapVolume"));
	RootComponent = OverlapVolume;

	// Radio de 50cm según la especificación
	OverlapVolume->SetSphereRadius(50.0f);
	OverlapVolume->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	OverlapVolume->SetCollisionObjectType(ECollisionChannel::ECC_WorldDynamic);
	OverlapVolume->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
	OverlapVolume->SetCollisionResponseToChannel(
		ECollisionChannel::ECC_WorldDynamic, ECollisionResponse::ECR_Overlap);

	// No replicar: los puntos de cobertura son objetos de nivel estático
	bReplicates = false;
}

void AZRCoverPoint::BeginPlay()
{
	Super::BeginPlay();

	// Asegurarse de que el estado inicial sea desocupado
	bIsOccupied = false;
	OccupyingCharacter = nullptr;
}

// ------------------------------------------------------------
// OCUPACION
// ------------------------------------------------------------

bool AZRCoverPoint::TryOccupy(APMCCharacter* Character)
{
	if (!Character)
	{
		return false;
	}

	// Si ya está ocupado por otro PMC, rechazar
	if (bIsOccupied && OccupyingCharacter != Character)
	{
		return false;
	}

	// Marcar como ocupado
	bIsOccupied = true;
	OccupyingCharacter = Character;

	return true;
}

void AZRCoverPoint::Release()
{
	// Liberar la cobertura para que otros PMCs puedan usarla
	bIsOccupied = false;
	OccupyingCharacter = nullptr;
}

// ------------------------------------------------------------
// VALIDACION DE COBERTURA
// ------------------------------------------------------------

bool AZRCoverPoint::CanProviderCoverFrom(FVector ThreatLocation) const
{
	// Obtener la posición de esta cobertura (ligeramente elevada para simular el cuerpo del PMC)
	FVector CoverPosition = GetActorLocation() + FVector(0.0f, 0.0f, 50.0f);

	// Configurar parámetros del trace
	FCollisionQueryParams TraceParams(FName("CoverValidation"), false);
	TraceParams.AddIgnoredActor(this);

	// Si hay un PMC ocupando esta cobertura, ignorarlo en el trace
	if (OccupyingCharacter)
	{
		TraceParams.AddIgnoredActor(OccupyingCharacter);
	}

	// Realizar sphere trace desde la amenaza hasta la posición de cobertura
	// Un sphere trace es más robusto que un line trace para detectar obstrucciones de geometría
	FHitResult HitResult;
	float TraceRadius = 20.0f; // Radio del trace en centímetros

	bool bHit = GetWorld()->SweepSingleByChannel(
		HitResult,
		ThreatLocation,
		CoverPosition,
		FQuat::Identity,
		ECollisionChannel::ECC_Visibility,
		FCollisionShape::MakeSphere(TraceRadius),
		TraceParams
	);

	if (bHit)
	{
		// Si el trace impactó algo antes de llegar a la cobertura, hay obstrucción = cobertura válida
		// Verificar que el impacto no sea en el propio actor de cobertura
		if (HitResult.GetActor() != this)
		{
			// Hay un obstáculo entre la amenaza y la cobertura: es cobertura válida
			return true;
		}
	}

	// No hay obstrucción: este punto no proporciona cobertura real contra la amenaza
	return false;
}

// ------------------------------------------------------------
// EDITOR
// ------------------------------------------------------------

#if WITH_EDITOR
void AZRCoverPoint::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);

	// Normalizar las direcciones de asomada para que sean vectores unitarios
	if (!PeekDirectionLeft.IsNearlyZero())
	{
		PeekDirectionLeft.Normalize();
	}
	if (!PeekDirectionRight.IsNearlyZero())
	{
		PeekDirectionRight.Normalize();
	}
}
#endif
