// InteractionComponent.cpp
// Implementación del componente de interacción para ZonaRoja

#include "Components/InteractionComponent.h"
#include "Camera/CameraComponent.h"
#include "Engine/World.h"
#include "DrawDebugHelpers.h"
#include "GameFramework/Actor.h"

UInteractionComponent::UInteractionComponent()
{
	PrimaryComponentTick.bCanEverTick = false;

	// Configuración por defecto de la interacción
	InteractionRange = 250.0f;     // 2.5 metros de alcance de interacción
	TraceRadius = 15.0f;           // Radio de la esfera de traza
	TraceTickRate = 0.1f;          // Actualización 10 veces por segundo
	InteractionChannel = ECC_GameTraceChannel2; // Canal "Interaction" definido en DefaultEngine.ini
}

void UInteractionComponent::BeginPlay()
{
	Super::BeginPlay();

	// Buscar la cámara del propietario para usarla como origen del raycast
	if (AActor* Owner = GetOwner())
	{
		OwnerCamera = Owner->FindComponentByClass<UCameraComponent>();
	}
}

void UInteractionComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	DisableInteraction();
	Super::EndPlay(EndPlayReason);
}

// ============================================================
// FUNCIONES PRINCIPALES
// ============================================================

void UInteractionComponent::EnableInteraction()
{
	// Iniciar el timer periódico de traza
	if (!GetWorld()->GetTimerManager().IsTimerActive(InteractionTraceTimer))
	{
		GetWorld()->GetTimerManager().SetTimer(InteractionTraceTimer, this,
			&UInteractionComponent::PerformInteractionTrace, TraceTickRate, true);
	}
}

void UInteractionComponent::DisableInteraction()
{
	GetWorld()->GetTimerManager().ClearTimer(InteractionTraceTimer);

	// Limpiar el objetivo actual si había uno
	if (CurrentInteractableActor)
	{
		CurrentInteractableActor = nullptr;
		CurrentInteractionPrompt = FText::GetEmpty();
		OnInteractableLost.Broadcast();
	}
}

void UInteractionComponent::ForceUpdateTrace()
{
	PerformInteractionTrace();
}

// ============================================================
// TRAZA DE INTERACCION
// ============================================================

void UInteractionComponent::PerformInteractionTrace()
{
	FVector TraceStart, TraceEnd;
	if (!GetTraceStartAndEnd(TraceStart, TraceEnd))
	{
		return; // No hay cámara disponible
	}

	// Configurar los parámetros de la traza
	FCollisionQueryParams QueryParams;
	QueryParams.AddIgnoredActor(GetOwner()); // Ignorar al propio personaje
	QueryParams.bTraceComplex = false;

	FHitResult HitResult;
	bool bHit = false;

	if (TraceRadius > 0.0f)
	{
		// Traza de esfera para mayor facilidad de interacción
		bHit = GetWorld()->SweepSingleByChannel(HitResult, TraceStart, TraceEnd,
			FQuat::Identity, InteractionChannel,
			FCollisionShape::MakeSphere(TraceRadius), QueryParams);
	}
	else
	{
		// Traza de línea estricta
		bHit = GetWorld()->LineTraceSingleByChannel(HitResult, TraceStart, TraceEnd,
			InteractionChannel, QueryParams);
	}

	// Debug visual en desarrollo (se puede desactivar en producción)
#if WITH_EDITOR
	const FColor DebugColor = bHit ? FColor::Green : FColor::Red;
	DrawDebugLine(GetWorld(), TraceStart, TraceEnd, DebugColor, false, TraceTickRate + 0.05f, 0, 1.0f);
#endif

	if (bHit && HitResult.GetActor())
	{
		AActor* HitActor = HitResult.GetActor();

		// Si es el mismo objeto que antes, no notificar de nuevo
		if (HitActor == CurrentInteractableActor)
		{
			return;
		}

		// Nuevo objeto encontrado: notificar pérdida del anterior si había
		if (CurrentInteractableActor)
		{
			OnInteractableLost.Broadcast();
		}

		// Actualizar el objetivo actual
		CurrentInteractableActor = HitActor;

		// Obtener el texto del prompt del actor (a través de una interfaz o etiqueta)
		// Por ahora usamos el nombre del actor; en la implementación final se usa IZRInteractable
		CurrentInteractionPrompt = FText::FromString(
			FString::Printf(TEXT("Interactuar con %s"), *HitActor->GetName()));

		OnInteractableFound.Broadcast(HitActor, CurrentInteractionPrompt);
	}
	else
	{
		// Ningún objeto en rango
		if (CurrentInteractableActor)
		{
			CurrentInteractableActor = nullptr;
			CurrentInteractionPrompt = FText::GetEmpty();
			OnInteractableLost.Broadcast();
		}
	}
}

bool UInteractionComponent::GetTraceStartAndEnd(FVector& OutStart, FVector& OutEnd) const
{
	// Usar la cámara del personaje como origen de la traza
	if (UCameraComponent* Camera = OwnerCamera.Get())
	{
		OutStart = Camera->GetComponentLocation();
		OutEnd = OutStart + (Camera->GetForwardVector() * InteractionRange);
		return true;
	}

	// Fallback: usar la posición y rotación del actor directamente
	if (AActor* Owner = GetOwner())
	{
		OutStart = Owner->GetActorLocation() + FVector(0.0f, 0.0f, 70.0f); // Altura de los ojos
		OutEnd = OutStart + (Owner->GetActorForwardVector() * InteractionRange);
		return true;
	}

	return false;
}
