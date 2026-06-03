// Archivo de construcción del módulo principal de ZonaRoja
// Define las dependencias del módulo para el sistema de construcción de Unreal

using UnrealBuildTool;

public class ZonaRoja : ModuleRules
{
    public ZonaRoja(ReadOnlyTargetRules Target) : base(Target)
    {
        PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

        // Módulos públicos: disponibles para otros módulos que dependan de éste
        PublicDependencyModuleNames.AddRange(new string[] {
            "Core",
            "CoreUObject",
            "Engine",
            "InputCore",
            "EnhancedInput",           // Sistema de entrada mejorado de UE5
            "GameplayTags",            // Etiquetas de gameplay (parte del motor, sin plugin extra)
            "NetCore",                 // Net/UnrealNetwork.h para replicación
            "UMG",                     // Blueprint/UserWidget, DragDropOperation
            "Slate",                   // Input/Reply.h y Framework de UI
            "SlateCore",               // Núcleo de Slate
            "Niagara",                 // NiagaraFunctionLibrary para VFX
            "PhysicsCore",             // PhysicalMaterials/PhysicalMaterial.h
            "NavigationSystem",        // NavigationSystem.h, PathFollowingComponent
            "AIModule"                 // AIController, BehaviorTree, Perception
        });

        // Módulos privados: solo usados internamente por este módulo
        PrivateDependencyModuleNames.AddRange(new string[] {
            "MotionWarping"            // MotionWarpingComponent (plugin habilitado en .uproject)
        });
    }
}
