// BTDecorator_CanSeeTarget.cpp
// Implementación del decorador de verificación de línea de visión

#include "AI/BTDecorators/BTDecorator_CanSeeTarget.h"
#include "AI/PMCCharacter.h"
#include "AI/PMCAIController.h"

#include "BehaviorTree/BehaviorTreeComponent.h"
#include "AIController.h"

UBTDecorator_CanSeeTarget::UBTDecorator_CanSeeTarget()
{
	// Nombre mostrado en el editor del BehaviorTree
	NodeName = TEXT("¿Puede Ver al Objetivo?");

	// Este decorador evalúa la condición sin necesitar tick continuo
	bNotifyBecomeRelevant = false;
	bNotifyCeaseRelevant = false;
}

bool UBTDecorator_CanSeeTarget::CalculateRawConditionValue(
	UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) const
{
	// Obtener el AIController del propietario del BT
	APMCAIController* AIController = Cast<APMCAIController>(OwnerComp.GetAIOwner());
	if (!AIController)
	{
		return false;
	}

	// Obtener el PMCCharacter del peón controlado
	APMCCharacter* PMCCharacter = Cast<APMCCharacter>(AIController->GetPawn());
	if (!PMCCharacter)
	{
		return false;
	}

	// PMC muerto nunca puede ver al objetivo
	if (PMCCharacter->CurrentState == EPMCState::Dead)
	{
		return false;
	}

	// Delegar la verificación al método de línea de visión del personaje
	// Este método realiza un trace que ignora a los compañeros PMCs
	return PMCCharacter->HasLineOfSightToPlayer();
}

FString UBTDecorator_CanSeeTarget::GetStaticDescription() const
{
	return TEXT("Condición: El PMC tiene línea de visión directa al jugador.\n"
		"Usa HasLineOfSightToPlayer() que ignora a los compañeros PMCs.");
}
