# GUÍA DE INICIO EN UNREAL ENGINE 5
## Proyecto ZonaRoja — Extraction Shooter
### Para principiantes absolutos en UE5

---

## ANTES DE EMPEZAR — Requisitos

Necesitas tener instalado:
- **Unreal Engine 5.3** (desde Epic Games Launcher)
- **Visual Studio 2022** (Community es gratis) con estas cargas de trabajo:
  - "Desarrollo de juegos con C++"
  - "Desarrollo de escritorio con C++"
- **Git** (para clonar el repositorio)

Si no tienes Visual Studio 2022, descárgalo de:
https://visualstudio.microsoft.com/es/vs/community/

---

## PASO 1 — CREAR EL PROYECTO C++ EN UE5

> ⚠️ Es OBLIGATORIO crear un proyecto C++, no Blueprint. Nuestro código fuente lo requiere.

1. Abre **Unreal Engine 5** desde Epic Games Launcher → botón **Launch**
2. Aparece la ventana "Unreal Project Browser"
3. En la columna izquierda, selecciona **Games**
4. De las plantillas, selecciona **Third Person** (tiene una cápsula de personaje lista)
5. En la parte inferior de la ventana, configura:
   - **Project Type**: selecciona **C++** (NO Blueprint)
   - **Quality Preset**: Maximum Quality
   - **Starter Content**: NO marques esta casilla (añade archivos innecesarios)
   - **Raytracing**: déjalo como está
6. En **Project Name** escribe exactamente: `ZonaRoja`
7. En **Location** elige la carpeta donde quieres guardar el proyecto
   - Recomendado: `C:\Proyectos\` o `D:\Proyectos\`
   - **IMPORTANTE**: La ruta NO debe tener espacios ni caracteres especiales
8. Haz clic en **Create**

UE5 abrirá Visual Studio automáticamente y comenzará a compilar.
Esto puede tardar **5-15 minutos** la primera vez. Es normal.

---

## PASO 2 — PRIMERA VEZ EN EL EDITOR

Cuando UE5 termine de cargar verás:
- **Viewport** (centro): vista 3D del nivel
- **Content Browser** (abajo): tus archivos y assets
- **Outliner** (derecha arriba): lista de objetos en el nivel
- **Details** (derecha abajo): propiedades del objeto seleccionado
- **Toolbar** (arriba): Play, Build, etc.

### Orientación básica en el Viewport
- **Clic derecho + W/A/S/D**: mover la cámara
- **Clic derecho + arrastrar**: rotar la cámara
- **Rueda del ratón**: zoom
- **F**: enfocar en el objeto seleccionado
- **Alt + G**: ocultar/mostrar cuadrícula

---

## PASO 3 — ACTIVAR LOS PLUGINS NECESARIOS

Los plugins añaden funcionalidades al motor. Nuestro juego los necesita.

1. En la barra superior ve a **Edit → Plugins**
2. Busca y activa cada uno de estos plugins (usa la barra de búsqueda):

| Plugin | Cómo buscarlo | Para qué sirve |
|--------|---------------|----------------|
| Enhanced Input | "Enhanced Input" | Sistema de controles moderno |
| Gameplay Abilities | "Gameplay Ab" | Sistema de habilidades (GAS) |
| Common UI | "CommonUI" | Menús y HUD avanzado |
| Niagara | "Niagara" | Efectos de partículas |
| MetaSounds | "MetaSound" | Audio procedural |
| Motion Warping | "Motion Warp" | Animaciones de vault/escalar |

Para activar cada uno: encuentra el plugin → marca la casilla a su izquierda → el botón cambia a azul.

3. Cuando hayas activado todos, haz clic en **Restart Now**
4. UE5 se reiniciará. Espera a que cargue de nuevo.

---

## PASO 4 — CONFIGURAR PROJECT SETTINGS

Aquí configuramos el rendimiento y comportamiento base del juego.

1. Ve a **Edit → Project Settings**
2. Sigue cada sección:

### Sección: Maps & Modes (izquierda, bajo "Project")
- **Default GameMode**: déjalo por ahora, lo cambiaremos cuando tengamos el Blueprint
- **Editor Startup Map**: ThirdPersonMap (la que viene por defecto)
- **Game Default Map**: ThirdPersonMap

### Sección: Engine → Rendering (izquierda, bajo "Engine")
Busca y configura:
- **Global Illumination Method**: Lumen
- **Reflection Method**: Lumen
- **Shadows**: Virtual Shadow Maps
- **Anti-Aliasing Method**: Temporal Super-Resolution (TSR)
- **Motion Blur**: **DESMARCA** la casilla (desactivar — molesta en tactical shooters)
- **Lens Flare**: **DESMARCA** la casilla
- **Bloom**: deja activado, pon Intensity en 0.6 más adelante en PostProcess

### Sección: Engine → Input (izquierda, bajo "Engine")
- **Default Input Component Class**: `EnhancedInputComponent`
- **Default Player Input Class**: `EnhancedPlayerInput`

3. Cierra Project Settings con la X

---

## PASO 5 — ORGANIZAR EL CONTENT BROWSER

El Content Browser es donde viven TODOS tus assets (modelos, texturas, sonidos, Blueprints).
Vamos a crear la estructura de carpetas del proyecto.

### Cómo crear carpetas
1. En el Content Browser (parte inferior), haz clic derecho en el área vacía
2. Selecciona **New Folder**
3. Escribe el nombre y presiona Enter

### Estructura a crear
Crea estas carpetas dentro de **Content/**. Las carpetas con → son subcarpetas:

```
Content/
├── _Core/
│     ├── GameModes/
│     ├── GameStates/
│     ├── PlayerStates/
│     └── PlayerControllers/
│
├── Characters/
│     ├── Player/
│     │     ├── Meshes/
│     │     ├── Blueprints/
│     │     ├── Animations/
│     │     └── Materials/
│     └── Enemies/
│           ├── PMC/
│           │     ├── Meshes/
│           │     ├── Blueprints/
│           │     └── Animations/
│           └── Shared/
│
├── Weapons/
│     ├── _Base/
│     ├── Rifles/
│     │     └── AK103/
│     └── Attachments/
│
├── Maps/
│     ├── Raid_Industrial/
│     ├── MainMenu/
│     └── Shared/
│           └── ExtractionZones/
│
├── Environment/
│     └── Modular/
│           ├── Industrial/
│           └── Urban/
│
├── Gameplay/
│     ├── AI/
│     │     ├── BehaviorTrees/
│     │     ├── Blackboards/
│     │     └── Controllers/
│     └── Items/
│
├── UI/
│     ├── HUD/
│     ├── Inventory/
│     └── Shared/
│
├── Audio/
│     ├── Weapons/
│     └── MetaSounds/
│
├── VFX/
│     ├── Weapons/
│     └── Impacts/
│
├── Data/
│     ├── DataTables/
│     └── DataAssets/
│
└── _Dev/
      └── TestMaps/
```

**Tip**: Para crear subcarpetas, navega primero a la carpeta padre haciendo doble clic en ella, luego clic derecho → New Folder.

---

## PASO 6 — COPIAR EL CÓDIGO C++ DEL REPOSITORIO

El código fuente ya está escrito y en Git. Ahora hay que copiarlo al proyecto UE5.

### 6.1 Clonar el repositorio (si no lo has hecho)
Abre una terminal (PowerShell o CMD) y ejecuta:
```
git clone https://github.com/sebvstiansoto/git-test.git C:\ZonaRoja-Git
```

### 6.2 Localizar los archivos de código
El repositorio tiene la carpeta `ZonaRoja/Source/ZonaRoja/` con todo el código C++.

### 6.3 Copiar el código al proyecto UE5
1. Navega a donde creaste el proyecto UE5, por ejemplo:
   `C:\Proyectos\ZonaRoja\Source\ZonaRoja\`
2. Verás archivos como `ZonaRoja.h`, `ZonaRoja.cpp`, `ZonaRoja.Build.cs`
3. Copia el contenido de estas carpetas del repositorio hacia tu proyecto:
   - `ZonaRoja-Git/ZonaRoja/Source/ZonaRoja/Core/` → `C:\Proyectos\ZonaRoja\Source\ZonaRoja\Core\`
   - `ZonaRoja-Git/ZonaRoja/Source/ZonaRoja/Characters/` → `C:\Proyectos\ZonaRoja\Source\ZonaRoja\Characters\`
   - `ZonaRoja-Git/ZonaRoja/Source/ZonaRoja/Components/` → `C:\Proyectos\ZonaRoja\Source\ZonaRoja\Components\`
   - `ZonaRoja-Git/ZonaRoja/Source/ZonaRoja/Weapons/` → `C:\Proyectos\ZonaRoja\Source\ZonaRoja\Weapons\`
   - `ZonaRoja-Git/ZonaRoja/Source/ZonaRoja/AI/` → `C:\Proyectos\ZonaRoja\Source\ZonaRoja\AI\`
   - `ZonaRoja-Git/ZonaRoja/Source/ZonaRoja/UI/` → `C:\Proyectos\ZonaRoja\Source\ZonaRoja\UI\`
   - `ZonaRoja-Git/ZonaRoja/Source/ZonaRoja/Data/ZRTypes.h` → `C:\Proyectos\ZonaRoja\Source\ZonaRoja\Data\ZRTypes.h`
4. También copia `ZonaRoja.Build.cs` y reemplaza el existente en el proyecto.

### 6.4 Regenerar los archivos de Visual Studio
Después de copiar los archivos:
1. Cierra UE5 completamente
2. Navega a la carpeta del proyecto: `C:\Proyectos\ZonaRoja\`
3. Haz clic derecho en `ZonaRoja.uproject`
4. Selecciona **Generate Visual Studio project files**
5. Espera a que termine (verás una ventana de progreso)
6. Abre `ZonaRoja.sln` con Visual Studio 2022

### 6.5 Compilar en Visual Studio
1. En Visual Studio, arriba a la derecha verás el menú desplegable de configuración
2. Cámbialo a **Development Editor**
3. En el menú superior: **Build → Build Solution** (o Ctrl+Shift+B)
4. Espera a que compile. La primera vez tarda 10-20 minutos.
5. Cuando diga "Build: 1 succeeded" en la barra inferior, puedes abrir UE5.

### 6.6 Abrir el proyecto compilado
1. Abre `ZonaRoja.uproject` con doble clic
2. UE5 detectará los nuevos archivos C++ y pedirá compilar
3. Haz clic en **Yes**

---

## PASO 7 — CREAR TU PRIMER NIVEL DE PRUEBA

Antes de hacer nada complejo, crea un nivel simple para probar.

1. En el menú superior: **File → New Level**
2. Selecciona **Basic** (tiene suelo, luz y cielo básicos)
3. Guárdalo: **File → Save Current Level As**
4. Navega a `Content/Maps/` en el diálogo
5. Nombra el nivel: `TestMap_Basic`
6. Clic en **Save**

---

## PASO 8 — CREAR EL BLUEPRINT DEL PERSONAJE

Los Blueprints son la capa visual sobre el código C++. Aquí es donde configuras
los valores, asignas meshes y conectas sistemas sin escribir código.

1. En el Content Browser, navega a `Content/Characters/Player/Blueprints/`
2. Haz clic derecho → **Blueprint Class**
3. En la ventana "Pick Parent Class", busca y selecciona `ZRPlayerCharacter`
   (esta es nuestra clase C++ del personaje)
4. Nombra el Blueprint: `BP_PlayerCharacter`
5. Haz doble clic en `BP_PlayerCharacter` para abrirlo

Dentro del Blueprint verás:
- **Viewport** (pestaña): vista 3D del personaje
- **Components** (izquierda): lista de componentes
- **Event Graph** (pestaña): lógica visual

Por ahora cierra el Blueprint. Lo configuraremos en el siguiente paso.

---

## PASO 9 — ASIGNAR EL ESQUELETO AL PERSONAJE

Para que el personaje se vea en pantalla, necesita un Skeletal Mesh.
UE5 viene con el "Mannequin" (maniquí de prueba).

1. Abre `BP_PlayerCharacter` (doble clic)
2. En el panel **Components** (izquierda), haz clic en **Mesh (CharacterMesh0)**
3. En el panel **Details** (derecha) busca la sección **Mesh**
4. Haz clic en el selector de **Skeletal Mesh Asset**
5. Busca "Manny" o "SKM_Manny" — es el maniquí que viene con la plantilla
6. Selecciónalo
7. El personaje aparecerá en el Viewport del Blueprint

Ajusta la posición del mesh:
- En Details, busca **Transform → Location**
- Pon Z = -90 (para que los pies queden en el suelo de la cápsula)
- Pon **Rotation** → Z = -90 (para que mire hacia adelante)

8. Haz clic en **Compile** (botón arriba a la izquierda con un rayo verde)
9. Haz clic en **Save**

---

## PASO 10 — CONFIGURAR EL GAMEMODE

El GameMode define las reglas básicas del juego: qué personaje usar, etc.

1. En `Content/_Core/GameModes/`, clic derecho → **Blueprint Class**
2. Busca y selecciona `RaidGameMode` como clase padre
3. Nombra: `BP_RaidGameMode`
4. Abre el Blueprint con doble clic
5. En el panel **Details** busca la sección **Classes**:
   - **Default Pawn Class**: selecciona `BP_PlayerCharacter`
   - **Player Controller Class**: selecciona `RaidPlayerController` (o su BP cuando lo crees)
6. Haz clic en **Compile** y **Save**

### Asignar el GameMode al nivel
1. Asegúrate de tener abierto `TestMap_Basic`
2. Ve al menú: **Window → World Settings** (se abre un panel a la derecha)
3. En **World Settings**, busca **Game Mode Override**
4. Selecciona `BP_RaidGameMode`

---

## PASO 11 — PROBAR EL JUEGO POR PRIMERA VEZ

1. En la barra superior del editor, haz clic en el botón **Play** (triángulo verde)
2. El juego comenzará dentro del editor
3. Deberías ver al personaje en el nivel
4. Prueba moverse con WASD, rotar con el ratón

**Controles por defecto** (configurados en el sistema Enhanced Input):
- W/A/S/D: moverse
- Ratón: mirar
- Shift: correr
- C: agacharse
- Espacio: saltar
- Tab: abrir inventario (cuando esté configurado)

Para salir del modo Play: presiona **Escape** o **Shift+F1** para recuperar el cursor.

---

## SOLUCIÓN DE PROBLEMAS COMUNES

### "No se puede compilar" / Errores de compilación
1. Cierra UE5
2. Elimina las carpetas `Binaries/` e `Intermediate/` del proyecto
3. Haz clic derecho en el `.uproject` → Generate Visual Studio project files
4. Vuelve a compilar desde Visual Studio

### "Missing module" al abrir el proyecto
- El código C++ no está compilado. Sigue el Paso 6.5.

### El personaje no aparece en el nivel
- Comprueba que `BP_RaidGameMode` está asignado en World Settings
- Comprueba que `Default Pawn Class` en el GameMode apunta a `BP_PlayerCharacter`

### Crash al iniciar Play
- Abre el **Output Log**: Window → Output Log
- Busca líneas en rojo que digan "Error"
- Comparte el error para ayudarte a resolverlo

---

## GLOSARIO RÁPIDO

| Término UE5 | Qué es |
|-------------|--------|
| Actor | Cualquier objeto en el mundo (personaje, luz, trigger) |
| Blueprint (BP) | Script visual. Funciona sobre una clase C++ |
| Mesh | Modelo 3D. Static Mesh = sin animación, Skeletal Mesh = con esqueleto |
| Material | "Pintura" del modelo: colores, texturas, efectos |
| Widget | Elemento de UI (botón, barra de vida, etc.) |
| Pawn | Actor que puede ser controlado por un jugador o IA |
| Character | Pawn con movimiento humanado y cápsula de colisión |
| GameMode | Define las reglas del juego. Solo existe en el servidor |
| Component | Módulo añadido a un Actor (HealthComponent, InventoryComponent, etc.) |
| DataTable | Hoja de cálculo de propiedades de items, armas, etc. |
| Blackboard | "Memoria" de la IA. Almacena variables que lee el Behavior Tree |
| Behavior Tree | Árbol de decisiones de la IA |
| Nanite | Sistema de UE5 que optimiza automáticamente polígonos de mallas estáticas |
| Lumen | Sistema de iluminación global dinámica de UE5 |

---

## PRÓXIMOS PASOS (en orden)

1. ✅ Crear proyecto C++ y activar plugins
2. ✅ Organizar Content Browser
3. ✅ Copiar código fuente y compilar
4. ✅ Crear BP_PlayerCharacter y asignar mesh
5. ✅ Crear BP_RaidGameMode y probar el juego
6. ⬜ Importar primer modelo desde Blender (ver BLENDER_TO_UE5_PIPELINE.md)
7. ⬜ Crear el DataTable DT_ItemDefinitions con los items del juego
8. ⬜ Crear el Blueprint del inventario (WBP_InventoryMain)
9. ⬜ Crear el primer mapa de raid (blockout en Blender o con BSP en UE5)
10. ⬜ Configurar el Behavior Tree de la IA PMC
