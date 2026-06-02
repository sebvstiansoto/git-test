# PIPELINE BLENDER → UNREAL ENGINE 5
## Flujo de Trabajo Optimizado para ZONA ROJA

**Versión:** 1.0  
**Blender:** 4.x  
**UE5:** 5.3+  

---

## 1. CONFIGURACIÓN INICIAL DE BLENDER

### 1.1 Unidades y Escala

La escala es la fuente de error más frecuente al exportar de Blender a UE5.

**Configuración correcta en Blender:**
```
Scene Properties → Units:
  Unit System:   Metric
  Unit Scale:    0.01  ← MUY IMPORTANTE
  Length:        Centimeters
```

Con `Unit Scale = 0.01`, **1 unidad de Blender = 1 cm en UE5**. Esto es lo correcto porque:
- UE5 trabaja en centímetros internamente (1 UE unit = 1 cm)
- Un personaje humano mide ~180 unidades en Blender → 180 cm en UE5

**Referencia de tamaños:**
```
Personaje humano:   1.75-1.85 unidades (175-185 cm en UE5)
Puerta estándar:    2.10 unidades de alto × 0.90 ancho
Techo estándar:     2.80 - 3.20 unidades
Pared modular:      3.00 × 3.00 (módulo base)
AK-103 (longitud):  0.94 unidades
```

### 1.2 Configuración de FPS y Timeline (para animación)

```
Output Properties → Frame Rate: 30 FPS
                    Frame Range: según animación
Scene Properties → Gravity: Z = -980 cm/s² (si se usa simulación física)
```

### 1.3 Overlays y Helpers Recomendados

```
Viewport Overlays → Statistics: Activado (ver vértices/tris en tiempo real)
Viewport Overlays → Edge Length: Activado (para UV seams)
Preferences → System → Memory & Limits → Undo Steps: 64
```

---

## 2. FLUJO DE TRABAJO PARA ASSETS MODULARES DE ENTORNO

### 2.1 Filosofía Modular

Todo el entorno se construye con piezas que encajan en una cuadrícula de 100cm (1 unidad Blender = 100 cm UE5):

```
Módulo base:     100 × 100 × 100 cm
Módulo doble:    200 × 100 × 100 cm
Módulo muro:     100 × 300 × 25 cm (largo × alto × grosor)
Módulo suelo:    100 × 100 × 10 cm
Módulo techo:    100 × 100 × 10 cm
```

**Regla de pivote:** El pivote (origen) de cada malla modular debe estar siempre en una esquina inferior, alineado con la cuadrícula. Esto permite snap perfecto en UE5.

```
Muro:   pivote en esquina inferior izquierda (X=0, Y=0, Z=0 del bounding box)
Suelo:  pivote en esquina inferior izquierda en el plano horizontal
Pilar:  pivote centrado en la base (X=0, Y=0 centro; Z=0 base)
Techo:  pivote alineado con la parte inferior del mesh
```

Para mover el pivote en Blender:
```
1. Object Mode: cursor 3D al punto deseado (Shift+Click derecho)
2. Object Properties → Origin → Origin to 3D Cursor
```

### 2.2 Proceso de Creación de Set Modular

1. **Crear pieza prototipo en grey mesh** (sin UVs, sin materiales)
2. **Verificar snap en cuadrícula** (escalar al importar en UE5 Level Editor)
3. **Añadir detalle geométrico** con el poly budget correcto
4. **Crear UV Channel 0** (texturas, colores, detalles)
5. **Crear UV Channel 1** (lightmap — ver sección UV)
6. **Añadir material slots** con nombres de convenio (ver sección materiales)
7. **Crear LODs** (ver sección LODs)
8. **Crear malla de colisión** (ver sección colisiones)
9. **Exportar FBX** con ajustes correctos

### 2.3 Naming Convention para Mallas Modulares

```
SM_[Set]_[Tipo]_[Variante]

Ejemplos:
SM_Industrial_Wall_Straight
SM_Industrial_Wall_Corner_90
SM_Industrial_Wall_Window
SM_Industrial_Floor_Clean
SM_Industrial_Floor_Damaged
SM_Industrial_Pillar_Base
SM_Industrial_Door_Frame
SM_Urban_Wall_Brick_A
SM_Urban_Wall_Brick_B
SM_Urban_Ceiling_Concrete
```

---

## 3. CREACIÓN DE LODs EN BLENDER

### 3.1 Presupuesto de Polígonos por LOD

#### Entorno Modular
| LOD | Distancia de activación | Reducción |
|-----|------------------------|-----------|
| LOD0 | 0 - 20m | 100% (máx calidad) |
| LOD1 | 20 - 50m | 50% de LOD0 |
| LOD2 | 50 - 100m | 25% de LOD0 |
| LOD3 | 100m+ | 10% de LOD0 |

#### Chars y Armas
| LOD | Distancia | Tris máx |
|-----|-----------|----------|
| LOD0 | 0 - 10m | Personaje: 25K, Arma: 15K |
| LOD1 | 10 - 30m | 50% LOD0 |
| LOD2 | 30 - 60m | 25% LOD0 |
| LOD3 | 60m+ | 10% LOD0 (o CULLED) |

### 3.2 Crear LODs en Blender (método Decimate + manual cleanup)

```
Para cada malla LOD0:

1. Duplicar el objeto: Shift+D → Enter (misma posición)
2. Renombrar: SM_Industrial_Wall_Straight_LOD1
3. En Edit Mode, aplicar Decimate Modifier:
   - Ratio: 0.5 para LOD1, 0.25 para LOD2
   - Type: Collapse
   - Triangulate: ON
4. Revisar y corregir manualmente:
   - Eliminar triángulos internos invisibles
   - Preservar silueta
   - Asegurarse que no hay vértices sueltos
5. Mantener el mismo nombre de material que LOD0
```

**Para Nanite (entorno estático):** Si el mesh usará Nanite en UE5, solo necesitas LOD0. Nanite genera sus propios niveles de detalle internamente. Sin embargo, crea un LOD1 simple como fallback para plataformas sin Nanite.

### 3.3 Convención de Nombres para LODs en el FBX

Al exportar, los LODs deben nombrarse así para que UE5 los detecte automáticamente:

```
SM_Industrial_Wall_Straight         ← LOD0 (nombre base)
SM_Industrial_Wall_Straight_LOD1    ← LOD1
SM_Industrial_Wall_Straight_LOD2    ← LOD2
SM_Industrial_Wall_Straight_LOD3    ← LOD3
```

En Blender, nombra los objetos exactamente así antes de exportar. Al importar el FBX en UE5 con `Import LODs = true`, los detectará automáticamente.

---

## 4. UV MAPPING

### 4.1 Canal UV 0 — Texturas

```
Propósito:   Texturas de color, normal, roughness, metallic
Límites:     0 a 1 (rango de textura)
Reglas:
  - Puede tener overlapping (UDIM si el asset lo requiere)
  - Aprovechar al máximo el espacio (texel density consistente)
  - Seams en bordes no visibles (interior de geometría, líneas de silhouette)
  - Sin estiramientos (stretch) en superficies planas > 0.5%
```

**Texel Density estándar del proyecto:**
```
Props hero (primer plano):  512 texels/m
Props secundarios:           256 texels/m
Fondo / entorno lejano:      128 texels/m
```

Para verificar texel density en Blender: instalar addon `TexTools` o usar el viewport de UV con checker texture.

### 4.2 Canal UV 1 — Lightmap

```
Propósito:   Bake de luz estática en UE5 (si se usa Lumen dinámico, aun así UE5 lo requiere)
Límites:     0 a 1 ESTRICTAMENTE (sin overlapping permitido)
Reglas CRÍTICAS:
  - Cada isla UV debe tener un margen mínimo de 2 píxeles entre islas
  - NINGUNA isla puede solaparse con otra
  - Todas las islas dentro del cuadrado 0-1
  - Las islas deben ser ortogonales o lo más rectas posible
```

**Crear UV Lightmap en Blender:**
```
1. En Edit Mode → UV Editor, añadir nuevo canal UV: Properties → Data → UV Maps → +
2. Nombrar el nuevo canal: "LightmapUV" o "UV1"
3. Con todo el mesh seleccionado, en UV Editor: UV → Smart UV Project
   - Angle Limit: 66°
   - Island Margin: 0.02 (2% de padding)
4. Verificar que no hay overlapping: UV → Seams from Islands, luego visual check
5. Ajustar islas manualmente para optimizar empaquetado
```

**Resolución de lightmap recomendada:**
```
Props pequeños:     64 × 64
Props medianos:     128 × 128
Muros y suelos:     256 × 256
Estructuras grandes: 512 × 512
Terreno:            1024 × 1024 (por chunk)
```

### 4.3 Herramienta: TexTools Addon (Blender)

Instalar TexTools (free, GitHub) para:
- Verificar texel density automáticamente
- Rectificar UVs con un clic
- Empaquetar UVs eficientemente
- Detectar overlapping islands

---

## 5. CONFIGURACIÓN DE EXPORTACIÓN FBX

### 5.1 Configuración de Export FBX para Static Meshes

```
File → Export → FBX (.fbx)

Include:
  [✓] Limit to Selected Objects (solo exportar lo seleccionado)
  [✓] Object Types: Mesh (y Armature si es skeletal)
  [ ] Custom Properties: OFF

Transform:
  Scale: 1.00  ← NO cambiar
  Apply Scalings: FBX All
  Forward: -Z Forward  ← CRÍTICO
  Up: Y Up             ← CRÍTICO
  [✓] Apply Unit
  [✓] Use Space Transform
  [ ] Apply Transform: OFF (para skeletal meshes mantener OFF)

Geometry:
  Smoothing: Face
  [✓] Export Subdivision Surface: OFF
  [✓] Apply Modifiers: ON (aplica todos los modifiers antes de exportar)
  [✓] Triangulate Faces: ON (UE5 triangula internamente, pero mejor hacerlo en Blender para control)
  [ ] Loose Edges: OFF
  [ ] Tangent Space: ON (para normales correctas)

Armature (solo skeletal):
  [✓] Add Leaf Bones: OFF  ← IMPORTANTE, evita huesos extra en UE5
  Primary Bone Axis: Y Axis
  Secondary Bone Axis: X Axis

Animation (solo si exportas animaciones):
  [✓] Baked Animation
  [✓] NLA Strips
  [✓] All Actions: OFF (exportar acción activa)
  Simplify: 1.0 (sin simplificación de curvas)
```

### 5.2 Configuración de Import en UE5

Al arrastrar el FBX al Content Browser de UE5:

```
Static Mesh Import Options:
  [✓] Import as Skeletal: OFF (para static)
  [✓] Generate Lightmap UVs: OFF (usamos el UV1 de Blender)
    → Si el lightmap UV de Blender es correcto, desactivar la generación automática
  LOD Import: Import all LODs
  Normal Import Method: Import Normals and Tangents
  Normal Generation Method: Built-in
  [✓] Auto Generate Collision: OFF (usamos colisiones nombradas de Blender)
  [✓] One Convex Hull Per UCX: OFF (si tienes múltiples _UCX_)
  Transform → Import Translation: 0,0,0
  Transform → Import Rotation: 0,0,0
  Transform → Import Uniform Scale: 1.0

Skeletal Mesh Import Options:
  [✓] Import Mesh: ON
  Skeleton: [seleccionar SK_Player_Skeleton si ya existe]
  [✓] Import Animations: ON (si el FBX tiene animaciones)
  Import Uniform Scale: 1.0
  [✓] Convert Scene: ON
  [✓] Force Front X Axis: OFF
```

---

## 6. NOMENCLATURA DE SLOTS DE MATERIAL

El nombre del slot de material en Blender determina cómo UE5 asigna materiales automáticamente.

### 6.1 Convención de Nombres

```
M_[Tipo]_[Variante]

Ejemplos para entorno:
  M_Metal_Painted_Green   ← Metal pintado verde
  M_Concrete_Damaged      ← Concreto dañado
  M_Wood_Planks           ← Madera de tablas
  M_Glass_Dirty           ← Vidrio sucio

Ejemplos para armas:
  M_Metal_Gunmetal        ← Metal oscuro de arma
  M_Polymer_Black         ← Plástico/polímero negro
  M_Wood_Stock            ← Madera de culata

Ejemplos para personajes:
  M_Skin_Face             ← Piel de cara
  M_Clothing_Tactical     ← Ropa táctica
  M_Gear_Vest             ← Equipo/chaleco
  M_Hair                  ← Cabello
```

### 6.2 Por Qué Importa

En UE5, al importar el FBX, el editor crea automáticamente material slots con esos nombres. Si ya tienes materiales en Content Browser con esos nombres, UE5 los asignará automáticamente (usando la regla de nombre idéntico). Esto acelera enormemente la asignación de materiales en producción.

**Crear una lista maestra de materiales del proyecto** en un DataTable o spreadsheet compartido con el equipo.

---

## 7. SETUP DE SKELETAL MESH PARA PERSONAJES

### 7.1 Jerarquía de Huesos (Armature)

```
root
└── pelvis
    ├── spine_01
    │   └── spine_02
    │       └── spine_03
    │           ├── neck_01
    │           │   └── head
    │           │       ├── eye_l
    │           │       └── eye_r
    │           ├── clavicle_l
    │           │   └── upperarm_l
    │           │       └── lowerarm_l
    │           │           └── hand_l
    │           │               ├── thumb_01_l → thumb_02_l → thumb_03_l
    │           │               ├── index_01_l → index_02_l → index_03_l
    │           │               ├── middle_01_l → ...
    │           │               ├── ring_01_l → ...
    │           │               └── pinky_01_l → ...
    │           └── clavicle_r (espejo de clavicle_l)
    ├── thigh_l
    │   └── calf_l
    │       └── foot_l
    │           └── ball_l
    └── thigh_r (espejo de thigh_l)
```

**Convención de nombres:** Usar el sistema de Epic/Unreal Mannequin para compatibilidad con Retargeter de UE5. Esto permite reutilizar animaciones del Marketplace y del propio Epic.

```
Nombres Epic Mannequin (obligatorios para retargeting):
  root, pelvis, spine_01, spine_02, spine_03, neck_01, head
  clavicle_l/r, upperarm_l/r, lowerarm_l/r, hand_l/r
  thigh_l/r, calf_l/r, foot_l/r, ball_l/r
```

### 7.2 Huesos de Arma (Weapon Socket)

Añadir hueso al esqueleto del personaje para el socket del arma:

```
hand_r
└── weapon_r     ← Socket donde se attachea el arma (IK target también)
```

En UE5, crear un Socket llamado `weapon_r` en el Skeletal Mesh Asset (desde el Skeleton Editor).

### 7.3 Huesos IK (Inverse Kinematics)

Para IK de pies y manos con el terreno:
```
ik_foot_l    ← Copia de foot_l para IK goal
ik_foot_r
ik_hand_l    ← IK para mano izquierda en arma (forestock)
ik_hand_r    ← IK para mano derecha (grip principal)
ik_foot_root ← Root de pies para full body IK
```

### 7.4 Weight Painting

Reglas de peso para evitar artefactos:
- Cada vértice debe tener al menos 1 influencia de hueso (peso = 1.0 en ese caso)
- Máximo 4 influencias por vértice (limitación de UE5 por defecto)
- Zona de articulaciones: gradiente suave, evitar pesos hard en articulaciones
- Ropa sobre cuerpo: los vértices de ropa deben seguir el hueso más cercano + blend suave

```
Object Mode → Data Properties → Vertex Groups → Normalize All (para asegurar que suman 1.0)
Object Mode → Data Properties → Vertex Groups → Limit Total: 4 (limitar a 4 influencias)
```

### 7.5 Rest Pose

El personaje debe estar en T-Pose o A-Pose en la rest pose de Blender:
- T-Pose: brazos horizontales, palmas hacia abajo
- A-Pose: brazos a 45° de los costados (más ergonómico para animación)
- **Usar A-Pose es la recomendación de Epic para UE5** (el Mannequin está en A-Pose)

---

## 8. COLISIONES — NOMENCLATURA UCX, UBX, USP

UE5 detecta automáticamente mallas de colisión dentro del FBX si siguen el naming convention correcto.

### 8.1 Tipos de Colisión

| Prefijo | Forma | Uso |
|---------|-------|-----|
| `UCX_` | Convex hull | Colisión general de formas complejas |
| `UBX_` | Box (caja) | Cajas y superficies planas |
| `USP_` | Sphere (esfera) | Objetos redondos, proyectiles |
| `USPC_` | Capsule | Raramente necesario (UE5 lo soporta) |

### 8.2 Convención de Nombre Completo

```
Formato: [Prefijo]_[NombreMeshBase]_[Índice]

Ejemplos:
  UCX_SM_Industrial_Wall_Straight_00  ← Colisión convexa del muro
  UCX_SM_Industrial_Wall_Straight_01  ← Segunda colisión convexa (si hay 2)
  UBX_SM_Crate_Wood_00               ← Caja de madera, colisión box
  USP_SM_Tank_Barrel_00              ← Barril esférico
```

### 8.3 Cómo Crear Colisiones en Blender

**Para UCX (convex hull):**
```
1. Duplicar el mesh base (solo las partes visibles relevantes)
2. Simplificar a low poly (sin curvas, sin detalles menores)
3. La forma debe ser CONVEXA (sin concavidades)
4. Object Properties → Name: UCX_[NombreMesh]_00
5. Al exportar, asegurarse de incluir este objeto en el FBX
```

**Para formas no-convexas (edificio completo):**
```
Dividir en múltiples UCX que juntos aproximen la forma:
  UCX_SM_Building_A_00  ← Planta baja izquierda
  UCX_SM_Building_A_01  ← Planta baja derecha
  UCX_SM_Building_A_02  ← Primer piso
  UCX_SM_Building_A_03  ← Tejado
```

### 8.4 Mallas de Colisión — Reglas

- El mesh de colisión NO necesita UV
- Puede estar completamente separado del mesh visual
- Polígonos: lo más bajo posible (50-200 tris para objetos normales)
- Asegurarse de que no hay agujeros o faces inversas en la colisión
- En UE5: probar siempre con `Show Collision` en el viewport

---

## 9. OPTIMIZACIÓN DE TEXTURAS

### 9.1 Formatos de Compresión en UE5

| Tipo de textura | Formato UE5 | Bits | Notas |
|-----------------|-------------|------|-------|
| Color/Albedo (RGB) | BC7 | 8 bpc | Mejor calidad, reemplaza DXT5 |
| Color con alpha (RGBA) | BC7 | 8 bpc | Transparency, UI |
| Normal Map | BC5 (RG) | 8 bpc por canal | Solo R y G, B se reconstruye |
| Roughness/Metallic/AO packed | BC7 o BC4 | 8/4 bpc | Pack en canales RGB |
| Mask (1 canal) | BC4 | 8 bpc | Opacidad, máscara |
| HDR/Emissive | BC6H | 16 bpc float | Emisivos, skybox |

**En UE5 Texture Settings:**
```
Albedo:     Compression = Default (BC7)
Normal:     Compression = Normalmap (BC5)
ORM:        Compression = Masks (BC4/BC7)
Emissive:   Compression = HDR (BC6H)
UI:         Compression = UserInterface2D (BC7, no mipmaps)
```

### 9.2 Resoluciones Máximas por Categoría

| Categoría | Resolución max | Resolución estándar |
|-----------|----------------|---------------------|
| Personaje hero (cara/manos) | 4096×4096 | 2048×2048 |
| Personaje hero (cuerpo) | 2048×2048 | 2048×2048 |
| Arma hero (primer plano) | 2048×2048 | 2048×2048 |
| Props de entorno visibles | 2048×2048 | 1024×1024 |
| Mods/tileables de entorno | 1024×1024 | 512×512 |
| Decals | 1024×1024 | 512×512 |
| UI elementos | 512×512 | 256×256 |
| Iconos de inventario | 256×256 | 128×128 |

**Regla:** Las texturas siempre deben ser potencia de 2 (512, 1024, 2048, 4096). Las texturas no-cuadradas deben ser potencia de 2 en cada dimensión (512×1024 OK, 500×1000 NO).

### 9.3 Texture Packing (ORM Map)

Para reducir el número de texturas, empaquetar en un solo archivo ORM:

```
Canal R (Rojo):    Oclusión Ambiental (AO)
Canal G (Verde):   Roughness
Canal B (Azul):    Metallic

Exportar desde Blender/Substance como: T_[Asset]_ORM.png
En UE5: Compression = Masks, sRGB = OFF
```

### 9.4 Preparación de Texturas en Blender/Substance

**Para Albedo:**
- Espacio de color: sRGB
- Sin información de iluminación bakeada (iluminación en UE5 es dinámica con Lumen)
- Solo color base del material

**Para Normal Map:**
- Espacio de color: Linear (Non-Color Data en Blender)
- Formato Blender: OpenGL (+Y hacia arriba)
- UE5 usa DirectX (–Y), marcar en UE5: `Flip Green Channel = true` si el normal parece invertido

---

## 10. ASSETS NANITE-READY

### 10.1 Qué Hace a un Mesh Ideal para Nanite

Nanite es ideal para:
- Mallas con alta densidad de polígonos
- Sin animaciones (static meshes)
- Sin deformaciones de vértice en material (World Position Offset complejo)

**Requisitos del mesh para Nanite:**
```
✓ Sin triángulos degenerados (área = 0)
✓ Sin vértices duplicados en el mismo punto
✓ Sin normales invertidas en superficies visibles
✓ El mesh debe ser "watertight" (cerrado) o con bordes bien definidos
✓ Triángulos bien proporcionados (evitar triángulos muy agudos o muy elongados)
✗ NO: Skeletal Meshes
✗ NO: Materiales translúcidos o masked con clip dinámico complejo
✗ NO: World Position Offset que deforme el mesh significativamente
```

### 10.2 Activar Nanite en UE5

Al importar el Static Mesh, en el Static Mesh Editor:
```
Details → Nanite Settings → Enable Nanite Support: ON
Nanite Settings → Position Precision: Auto
Nanite Settings → Target Relative Error: 0.0 (calidad máxima) o 0.1 (ahorro)
Nanite Settings → Fallback Relative Error: 1.0 (para plataformas sin Nanite)
```

### 10.3 Verificación Post-Import

Tras importar cada asset en UE5, verificar:
1. **Pivot correcto:** Actor en el viewport tiene el pivote donde se modeló
2. **Escala:** El mesh tiene el tamaño esperado (colocar un Mannequin de referencia)
3. **Normales:** Sin artefactos de iluminación extraños (activar `Lit` mode)
4. **Colisiones:** Activar `Show Collision` en viewport y verificar forma
5. **LODs:** En Static Mesh Editor → LODs, verificar que los LODs están presentes
6. **UV Lightmap:** UV Editor → Channel 1, verificar que no hay overlapping
7. **Materiales:** Los slots de material están asignados correctamente

---

## 11. RESUMEN DEL FLUJO COMPLETO

```
BLENDER                          →    UE5

1. Modelar en cm (Unit Scale 0.01)
2. Crear UV Channel 0 (texturas)
3. Crear UV Channel 1 (lightmap)
4. Crear LODs (LOD0-3)
5. Nombrar material slots (M_...)
6. Crear colisiones UCX/UBX/USP
7. Para skeletal: crear armature compatible
   con Epic Mannequin naming
8. Exportar FBX con ajustes correctos
   (Forward: -Z, Up: Y, Scale: 1.0)
                                       9. Importar FBX en UE5
                                      10. Verificar escala y pivot
                                      11. Asignar materiales
                                      12. Activar Nanite si aplica
                                      13. Configurar colisiones
                                      14. Verificar LODs
                                      15. Guardar y añadir a level
```

---

*Ver también BLENDER_ASSETS_PLAN.md para el plan de producción completo de assets.*
