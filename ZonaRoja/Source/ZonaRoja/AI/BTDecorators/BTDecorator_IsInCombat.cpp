// BTDecorator_IsInCombat.cpp
// Implementación del decorador de verificación de estado de combate

#include "AI/BTDecorators/BTDecorator_IsInCombat.h"
#include "AI/PMCCharacter.h"
#include "AI/PMCAIController.h"

#include "BehaviorTree/BehaviorTreeComponent.h"
#include "AIController.h"

UBTDecorator_IsInCombat::UBTDecorator_IsInCombat()
{
	// Nombre mostrado en el editor del BehaviorTree
	NodeName = TEXT("¿Está en Combate?");

	// Este decorador evalúa la condición puntualmente, sin tick
	bNotifyBecomeRelevant = false;
	bNotifyCeaseRelevant = false;
}

bool UBTDecorator_IsInCombat::CalculateRawConditionValue(
	UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) const
{
	// Obtener el AIController
	APMCAIController* AIController = Cast<APMCAIController>(OwnerComp.GetAIOwner());
	if (!AIController)
	{
		return false;
	}

	// Obtener el PMCCharacter
	APMCCharacter* PMCCharacter = Cast<APMCCharacter>(AIController->GetPawn());
	if (!PMCCharacter)
	{
		return false;
	}

	// Verificar que el estado actual sea exactamente Combat
	// (no Alert ni ningún otro estado relacionado con la amenaza)
	return PMCCharacter->CurrentState == EPMCState::Combat;
}

FString UBTDecorator_IsInCombat::GetStaticDescription() const
{
	return TEXT("Condición: El PMC está en estado EPMCState::Combat.\n"
		"Activa las ramas de comportamiento de combate activo.");
}
