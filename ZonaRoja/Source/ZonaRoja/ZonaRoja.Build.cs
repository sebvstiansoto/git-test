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
            "GameplayAbilities",       // Sistema de habilidades de gameplay (GAS)
            "GameplayTags",            // Etiquetas de gameplay para clasificación
            "GameplayTasks",           // Tareas de gameplay asíncronas
            "NetCore",                 // Núcleo de red para multijugador
            "OnlineSubsystem",         // Subsistema online abstracto
            "OnlineSubsystemUtils",    // Utilidades del subsistema online
            "UMG",                     // Unreal Motion Graphics para UI
            "Slate",                   // Framework de UI Slate
            "SlateCore",               // Núcleo de Slate
            "CommonUI",                // UI común reutilizable
            "Niagara",                 // Sistema de partículas Niagara para VFX
            "PhysicsCore",             // Física de núcleo para balística
            "NavigationSystem",        // Sistema de navegación para IA
            "AIModule"                 // Módulo de inteligencia artificial
        });

        // Módulos privados: solo usados internamente por este módulo
        PrivateDependencyModuleNames.AddRange(new string[] {
            "MetasoundEngine",         // Motor de MetaSounds para audio procedural
            "MotionWarping"            // Warping de movimiento para animaciones contextuales
        });
    }
}
