// BTTask_FireAtTarget.cpp
// Implementación del disparo hitscan con dispersión angular y daño por partes del cuerpo

#include "AI/BTTasks/BTTask_FireAtTarget.h"
#include "AI/PMCCharacter.h"
#include "AI/PMCAIController.h"
#include "Characters/Player/ZRPlayerCharacter.h"
#include "Components/HealthComponent.h"
#include "Data/ZRTypes.h"

#include "BehaviorTree/BehaviorTreeComponent.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "AIController.h"
#include "Kismet/GameplayStatics.h"
#include "DrawDebugHelpers.h"

UBTTask_FireAtTarget::UBTTask_FireAtTarget()
{
	// Nombre mostrado en el editor del BehaviorTree
	NodeName = TEXT("Disparar al Objetivo");

	// Esta tarea se completa en un solo frame
	bNotifyTaskFinished = false;
}

EBTNodeResult::Type UBTTask_FireAtTarget::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	// ---- 1. Obtener referencias ----

	APMCAIController* AIController = Cast<APMCAIController>(OwnerComp.GetAIOwner());
	if (!AIController)
	{
		return EBTNodeResult::Failed;
	}

	APMCCharacter* PMCCharacter = Cast<APMCCharacter>(AIController->GetPawn());
	if (!PMCCharacter)
	{
		return EBTNodeResult::Failed;
	}

	UBlackboardComponent* BBComp = OwnerComp.GetBlackboardComponent();
	if (!BBComp)
	{
		return EBTNodeResult::Failed;
	}

	// ---- 2. Verificar línea de visión ----

	if (!PMCCharacter->HasLineOfSightToPlayer())
	{
		// Sin línea de visión: no puede disparar
		UE_LOG(LogTemp, Verbose, TEXT("[BTTask_FireAtTarget] %s: Sin LOS al jugador"),
			*PMCCharacter->GetName());
		return EBTNodeResult::Failed;
	}

	// ---- 3. Verificar cadencia de fuego ----

	if (PMCCharacter->TimeSinceLastShot < PMCCharacter->StatConfig.FireRate)
	{
		// Aún en tiempo de enfriamiento entre disparos
		return EBTNodeResult::Failed;
	}

	// ---- 4. Obtener objetivo del Blackboard ----

	AZRPlayerCharacter* TargetPlayer = Cast<AZRPlayerCharacter>(
		BBComp->GetValueAsObject(APMCAIController::BB_TargetActor));

	if (!TargetPlayer || !TargetPlayer->IsAlive())
	{
		return EBTNodeResult::Failed;
	}

	// ---- 5. Calcular dirección de disparo con dispersión ----

	FVector MuzzleLocation = PMCCharacter->GetMuzzleLocation();

	// Dirección base hacia el pecho del jugador
	FVector TargetLocation = TargetPlayer->GetActorLocation() + FVector(0.0f, 0.0f, 50.0f);
	FVector ShotDirection = (TargetLocation - MuzzleLocation).GetSafeNormal();

	// Aplicar dispersión aleatoria en cono usando VRandCone
	// El ángulo del cono es la mitad de AccuracySpread convertida a radianes
	float HalfSpreadRadians = FMath::DegreesToRadians(PMCCharacter->StatConfig.AccuracySpread * 0.5f);
	FVector SpreadDirection = FMath::VRandCone(ShotDirection, HalfSpreadRadians);

	// ---- 6. Realizar hitscan (LineTrace) ----

	FVector TraceEnd = MuzzleLocation + (SpreadDirection * MaxTraceDistance);

	FCollisionQueryParams TraceParams(FName("PMC_FireTrace"), true, PMCCharacter);
	TraceParams.bTraceComplex = true;
	TraceParams.bReturnPhysicalMaterial = false;

	// Ignorar a todos los otros PMCs para evitar fuego amigo accidental
	TArray<AActor*> AllPMCs;
	UGameplayStatics::GetAllActorsOfClass(PMCCharacter->GetWorld(), APMCCharacter::StaticClass(), AllPMCs);
	for (AActor* OtherPMC : AllPMCs)
	{
		TraceParams.AddIgnoredActor(OtherPMC);
	}

	FHitResult HitResult;
	bool bHit = PMCCharacter->GetWorld()->LineTraceSingleByChannel(
		HitResult,
		MuzzleLocation,
		TraceEnd,
		ECollisionChannel::ECC_Pawn,
		TraceParams
	);

	// ---- 7. Aplicar daño si impactó al jugador ----

	if (bHit && HitResult.GetActor() == TargetPlayer)
	{
		// Determinar parte del cuerpo impactada por el hueso
		EBodyPart HitBodyPart = GetBodyPartFromBone(HitResult.BoneName);

		// Obtener el componente de salud del jugador
		UHealthComponent* PlayerHealthComp = TargetPlayer->HealthComponent;
		if (PlayerHealthComp && !PlayerHealthComp->IsDead())
		{
			// Aplicar daño con el tipo Bullet
			PlayerHealthComp->ApplyDamage(
				BaseDamagePerShot,
				HitBodyPart,
				EDamageType::Bullet,
				PMCCharacter,
				HitResult
			);

			UE_LOG(LogTemp, Verbose, TEXT("[BTTask_FireAtTarget] %s impactó al jugador en %s, daño: %.1f"),
				*PMCCharacter->GetName(),
				*HitResult.BoneName.ToString(),
				BaseDamagePerShot);
		}
	}

	// ---- 8. Reiniciar contador de tiempo entre disparos ----

	PMCCharacter->TimeSinceLastShot = 0.0f;

	// ---- 9. Notificar disparo (efectos visuales y sonido) ----
	// El multicast se implementaría aquí para sincronizar efectos en todos los clientes
	// Ejemplo: PMCCharacter->MulticastOnFire(MuzzleLocation, SpreadDirection);

	// Efecto visual de debug en desarrollo
#if WITH_EDITOR
	DrawDebugLine(
		PMCCharacter->GetWorld(),
		MuzzleLocation,
		bHit ? HitResult.Location : TraceEnd,
		bHit ? FColor::Red : FColor::Yellow,
		false, // bPersistentLines
		0.5f,  // LifeTime en segundos
		0,     // DepthPriority
		1.5f   // Grosor
	);
#endif

	return EBTNodeResult::Succeeded;
}

// ------------------------------------------------------------
// MAPEO DE HUESOS A PARTES DEL CUERPO
// ------------------------------------------------------------

EBodyPart UBTTask_FireAtTarget::GetBodyPartFromBone(const FName& BoneName) const
{
	// Mapear nombres de huesos del skeleton a partes del cuerpo del sistema de daño
	// Se asume naming convention estándar de UE5 Mannequin/Manny

	const FString BoneStr = BoneName.ToString().ToLower();

	// Cabeza y cuello
	if (BoneStr.Contains(TEXT("head")) || BoneStr.Contains(TEXT("neck")))
	{
		return EBodyPart::Head;
	}

	// Pecho y espalda (torso superior)
	if (BoneStr.Contains(TEXT("spine_03")) || BoneStr.Contains(TEXT("spine_04")) ||
		BoneStr.Contains(TEXT("clavicle")) || BoneStr.Contains(TEXT("chest")))
	{
		return EBodyPart::Chest;
	}

	// Estómago (torso inferior)
	if (BoneStr.Contains(TEXT("spine_01")) || BoneStr.Contains(TEXT("spine_02")) ||
		BoneStr.Contains(TEXT("pelvis")) || BoneStr.Contains(TEXT("stomach")))
	{
		return EBodyPart::Stomach;
	}

	// Brazo izquierdo (upperarm, lowerarm, hand izquierdo)
	if (BoneStr.Contains(TEXT("upperarm_l")) || BoneStr.Contains(TEXT("lowerarm_l")) ||
		BoneStr.Contains(TEXT("hand_l")) || (BoneStr.Contains(TEXT("arm")) && BoneStr.Contains(TEXT("_l"))))
	{
		return EBodyPart::LeftArm;
	}

	// Brazo derecho
	if (BoneStr.Contains(TEXT("upperarm_r")) || BoneStr.Contains(TEXT("lowerarm_r")) ||
		BoneStr.Contains(TEXT("hand_r")) || (BoneStr.Contains(TEXT("arm")) && BoneStr.Contains(TEXT("_r"))))
	{
		return EBodyPart::RightArm;
	}

	// Pierna izquierda (thigh, calf, foot izquierdo)
	if (BoneStr.Contains(TEXT("thigh_l")) || BoneStr.Contains(TEXT("calf_l")) ||
		BoneStr.Contains(TEXT("foot_l")) || (BoneStr.Contains(TEXT("leg")) && BoneStr.Contains(TEXT("_l"))))
	{
		return EBodyPart::LeftLeg;
	}

	// Pierna derecha
	if (BoneStr.Contains(TEXT("thigh_r")) || BoneStr.Contains(TEXT("calf_r")) ||
		BoneStr.Contains(TEXT("foot_r")) || (BoneStr.Contains(TEXT("leg")) && BoneStr.Contains(TEXT("_r"))))
	{
		return EBodyPart::RightLeg;
	}

	// Por defecto: impacto en el pecho (parte más común y central)
	return EBodyPart::Chest;
}

FString UBTTask_FireAtTarget::GetStaticDescription() const
{
	return FString::Printf(
		TEXT("Dispara al objetivo con hitscan.\n"
		"Verifica LOS y cadencia de fuego.\n"
		"Daño base: %.1f. Alcance: %.0f cm."),
		BaseDamagePerShot, MaxTraceDistance);
}
