# PLAN DE PRODUCCIÓN DE ASSETS EN BLENDER
## Proyecto: ZONA ROJA

**Versión:** 1.0  
**Herramientas:** Blender 4.x, Substance Painter/Designer, Marmoset Toolbag (preview)  

---

## 1. LISTA DE ASSETS PRIORITARIA

La producción se organiza por prioridad de impacto en el juego. Los assets de mayor visibilidad y uso directo del jugador se producen primero.

### Prioridad 1 — CRÍTICOS (necesarios para primer playtest jugable)

| Asset | Tipo | Bloqueante de | Semana |
|-------|------|---------------|--------|
| Personaje jugador (cuerpo completo) | Skeletal Mesh | Movimiento, animaciones, primer test | 1-3 |
| Brazos FPP (primera persona) | Skeletal Mesh | Visión en raid, test de armas | 2-4 |
| AK-103 | Skeletal Mesh | Sistema de armas, disparo | 2-4 |
| M4A1 | Skeletal Mesh | Segunda arma para pruebas | 3-5 |
| PMC Bot (enemigo base) | Skeletal Mesh | IA, combate | 4-6 |
| Kit modular industrial (básico) | Static Mesh (set) | Nivel de prueba, bloques de juego | 1-3 |
| Caja de loot genérica | Static Mesh | Sistema de loot | 2 |
| Botiquín pequeño (ítem 3D) | Static Mesh | Sistema de inventario | 2 |

### Prioridad 2 — IMPORTANTES (necesarios para alpha interna)

| Asset | Tipo | Semana |
|-------|------|--------|
| Pistola Glock 17 | Skeletal Mesh | 5-6 |
| MP5 | Skeletal Mesh | 5-7 |
| Casco (Level 2) | Static Mesh | 4 |
| Chaleco táctico | Static Mesh | 4 |
| Mochila táctica | Static Mesh | 4 |
| Kit modular urbano (básico) | Static Mesh (set) | 5-7 |
| Personaje PMC variante (Veteran) | Skeletal Mesh | 7-8 |
| Props industriales (10 props) | Static Mesh | 5-7 |
| Vehículo abandonado (coche) | Static Mesh | 6-8 |
| Supresor 7.62 (adjunto) | Static Mesh | 5 |
| Mira ACOG (adjunto) | Static Mesh | 5 |

### Prioridad 3 — DESEABLES (para beta/demo vertical slice)

| Asset | Tipo | Semana |
|-------|------|--------|
| SVD (francotirador) | Skeletal Mesh | 8-10 |
| FAL (rifle de batalla) | Skeletal Mesh | 8-10 |
| Mossberg 590 (escopeta) | Skeletal Mesh | 9-11 |
| Mutante (enemigo) | Skeletal Mesh | 9-11 |
| Boss Barkov | Skeletal Mesh | 12+ |
| Kit modular militar | Static Mesh (set) | 9-11 |
| Terreno exterior del mapa 1 | Terrain/Landscape | 8-10 |
| Efectos atmosféricos (niebla, polvo) | VFX helpers | 10-12 |
| Más armas (HK416, Vector, PKM) | Skeletal Mesh | 10+ |
| Skins de equipo adicionales | Materials/Meshes | 11+ |

---

## 2. PRESUPUESTO DE POLÍGONOS POR CATEGORÍA

### 2.1 Personajes

| Asset | LOD0 (tris) | LOD1 (tris) | LOD2 (tris) | LOD3 (tris) |
|-------|-------------|-------------|-------------|-------------|
| Personaje jugador (cuerpo) | 25,000 | 12,000 | 6,000 | 2,000 |
| Personaje jugador (cabeza) | 8,000 | 4,000 | 2,000 | 500 |
| Brazos FPP (sólo brazos) | 6,000 | 3,000 | — | — |
| PMC Bot Base | 18,000 | 9,000 | 4,500 | 1,500 |
| PMC Bot Veteran | 20,000 | 10,000 | 5,000 | 1,500 |
| Mutante | 12,000 | 6,000 | 3,000 | 1,000 |
| Boss Barkov | 30,000 | 15,000 | 7,500 | 3,000 |

**Nota:** Los personajes usan Skeletal Mesh, no Nanite. Estos presupuestos son estrictos.

### 2.2 Armas

| Asset | LOD0 TPP (tris) | LOD0 FPP (tris) | LOD1 TPP | LOD2 TPP |
|-------|----------------|----------------|----------|----------|
| AK-103 | 8,000 | 15,000 | 4,000 | 1,500 |
| M4A1 | 9,000 | 16,000 | 4,500 | 1,500 |
| HK416 | 9,500 | 16,500 | 4,750 | 2,000 |
| MP5 | 7,000 | 12,000 | 3,500 | 1,200 |
| Vector | 7,500 | 13,000 | 3,750 | 1,200 |
| SVD | 10,000 | 18,000 | 5,000 | 2,000 |
| Mossberg 590 | 6,500 | 11,000 | 3,250 | 1,000 |
| PKM | 12,000 | 20,000 | 6,000 | 2,500 |
| Glock 17 | 5,000 | 9,000 | 2,500 | 800 |
| Supresor (adjunto) | 800 | 1,500 | 400 | — |
| Mira ACOG | 600 | 1,200 | 300 | — |
| Red Dot | 400 | 800 | 200 | — |

**FPP vs TPP:** El modelo FPP tiene más polígonos porque es lo que el jugador ve todo el tiempo. El modelo TPP puede ser más simple.

### 2.3 Entorno Modular (por pieza)

| Tipo de pieza | Tris máx LOD0 | ¿Nanite? |
|---------------|---------------|---------|
| Muro recto (3m) | 500 | Sí |
| Muro con ventana | 800 | Sí |
| Muro con puerta | 700 | Sí |
| Suelo (1m×1m) | 200 | Sí |
| Techo | 200 | Sí |
| Pilar | 600 | Sí |
| Escalera (1 tramo) | 1,200 | Sí |
| Prop pequeño (caja) | 800 | Sí |
| Prop mediano (estantería) | 1,500 | Sí |
| Prop grande (maquinaria) | 3,000 | Sí |
| Vehículo abandonado | 8,000 | Sí |
| Barricada (sacos tierra) | 1,200 | Sí |

**Nota:** Con Nanite habilitado, los límites de polígonos para Static Meshes de entorno son orientativos. Nanite maneja la densidad en tiempo de ejecución. Aun así, no exceder 50,000 tris por pieza modular individual.

### 2.4 Items de Inventario (icono 3D)

| Categoría | Tris |
|-----------|------|
| Armas (miniaturas para inventario) | 500-1,000 |
| Kits médicos | 300-600 |
| Consumibles | 200-400 |
| Equipo (cascos, chalecos) | 400-800 |
| Materiales/crafteo | 100-300 |

---

## 3. RESOLUCIONES DE TEXTURAS POR CATEGORÍA

### 3.1 Personajes

| Asset | Albedo | Normal | ORM | Emissive |
|-------|--------|--------|-----|---------|
| Cara/Cabeza | 2048×2048 | 2048×2048 | 2048×2048 | — |
| Cuerpo/Torso | 2048×2048 | 2048×2048 | 2048×2048 | — |
| Extremidades (brazos/piernas) | 1024×1024 | 1024×1024 | 1024×1024 | — |
| Equipo táctico (chaleco, etc.) | 2048×2048 | 2048×2048 | 2048×2048 | — |

### 3.2 Armas

| Asset | Albedo | Normal | ORM | Emissive |
|-------|--------|--------|-----|---------|
| Arma principal (hero) | 2048×2048 | 2048×2048 | 2048×2048 | 512×512 (si aplica) |
| Arma secundaria | 1024×1024 | 1024×1024 | 1024×1024 | — |
| Adjuntos grandes (supresor, bípode) | 1024×512 | 1024×512 | 1024×512 | — |
| Adjuntos pequeños (miras simples) | 512×512 | 512×512 | 512×512 | — |

### 3.3 Entorno

| Asset | Tileable base | Normal tileable | ORM tileable | Unique detail |
|-------|--------------|-----------------|--------------|---------------|
| Suelo de hormigón | 1024×1024 | 1024×1024 | 1024×1024 | — |
| Muro de ladrillo | 1024×1024 | 1024×1024 | 1024×1024 | — |
| Metal industrial | 1024×1024 | 1024×1024 | 1024×1024 | — |
| Props hero (visibles de cerca) | 1024×1024 | 1024×1024 | 1024×1024 | 512×512 |
| Props secundarios | 512×512 | 512×512 | 512×512 | — |
| Vehículo | 2048×2048 | 2048×2048 | 2048×2048 | — |

**Formatos de exportación:** PNG 16-bit para normales, PNG 8-bit para el resto. Nunca exportar con compresión JPEG (pérdida en normales causa artefactos visibles).

---

## 4. CONVENCIONES DE NOMBRADO PARA TODOS LOS ASSETS

### 4.1 Archivos Blender (.blend)

```
[Tipo]_[Nombre]_[Variante].blend

Ejemplos:
  CHAR_Player_Base.blend           ← Personaje jugador base
  CHAR_PMC_Veteran.blend           ← PMC variante veteran
  WEAPON_AK103_v01.blend           ← AK-103, primera iteración
  ENV_Industrial_Wall_Set.blend    ← Toda la colección de muros industriales en un .blend
  PROP_Crate_Wood_A.blend
  ATTACH_Suppressor_762.blend
```

### 4.2 Meshes dentro de Blender

```
[Prefijo]_[NombreBase]_[LOD]

Prefijos:
  SM_    → Static Mesh (malla estática)
  SK_    → Skeletal Mesh (malla con esqueleto)
  UCX_   → Colisión convexa
  UBX_   → Colisión box
  USP_   → Colisión esfera

Ejemplos:
  SK_AK103                     ← Mesh principal (LOD0 implícito)
  SK_AK103_LOD1
  SK_AK103_LOD2
  UCX_SK_AK103_00              ← Colisión del AK-103
  SM_Ind_Wall_Straight_3m
  SM_Ind_Wall_Straight_3m_LOD1
  UCX_SM_Ind_Wall_Straight_3m_00
```

### 4.3 Materiales dentro de Blender

Los materiales en Blender deben nombrarse igual que los slots de material final en UE5:

```
M_[Tipo]_[Descripción]

Ejemplos:
  M_Metal_Gunmetal             ← Metal de arma (slot en AK-103, M4A1, etc.)
  M_Polymer_Black              ← Polímero negro (guardamanos, empuñadura)
  M_Wood_Stock                 ← Madera (culata de madera)
  M_Concrete_Damaged           ← Concreto dañado (entorno industrial)
  M_Metal_Rusty                ← Metal oxidado
  M_Glass_Dirty                ← Vidrio sucio
  M_Skin_Face                  ← Piel de personaje
  M_Clothing_Tactical          ← Ropa táctica
  M_Gear_Vest                  ← Chaleco táctico
```

### 4.4 Texturas (archivos de imagen)

```
T_[NombreAsset]_[Tipo].[ext]

Tipos:
  _Albedo     → Color base (sRGB)
  _Normal     → Mapa de normales (Linear)
  _ORM        → Oclusión(R) Roughness(G) Metallic(B) (Linear)
  _Emissive   → Mapa emisivo (sRGB o HDR)
  _Opacity    → Canal de opacidad (Linear)
  _Height     → Height map para parallax (Linear)
  _ID         → ID map para pintar en Substance (Linear)

Ejemplos:
  T_AK103_Albedo.png
  T_AK103_Normal.png
  T_AK103_ORM.png
  T_Player_Face_Albedo.png
  T_Player_Face_Normal.png
  T_Ind_Wall_Concrete_Albedo.png   ← Textura tileable
  T_Ind_Wall_Concrete_Normal.png
```

### 4.5 Bones del Armature (Esqueleto)

```
Usar naming convention de Epic Games Mannequin UE5 para compatibilidad:

root
pelvis
spine_01, spine_02, spine_03
neck_01, head
clavicle_l, clavicle_r
upperarm_l, upperarm_r
lowerarm_l, lowerarm_r
hand_l, hand_r
  → finger_thumb_01_l → _02 → _03
  → finger_index_01_l → _02 → _03
  → finger_middle_01_l → _02 → _03
  → finger_ring_01_l → _02 → _03
  → finger_pinky_01_l → _02 → _03
thigh_l, thigh_r
calf_l, calf_r
foot_l, foot_r
ball_l, ball_r

Huesos IK (añadir al final de la jerarquía):
ik_foot_root
ik_foot_l, ik_foot_r
ik_hand_root
ik_hand_l, ik_hand_r

Socket de arma:
weapon_r  (hijo de hand_r)
```

### 4.6 Colecciones de Blender (para organización)

```
Organizar los objetos en colecciones dentro de Blender:

📂 WEAPON_AK103
  ├── 📂 Meshes
  │   ├── SK_AK103 (LOD0)
  │   ├── SK_AK103_LOD1
  │   └── SK_AK103_LOD2
  ├── 📂 Collision
  │   └── UCX_SK_AK103_00
  ├── 📂 Armature
  │   └── AK103_Armature
  └── 📂 Reference
      └── Reference_Image_AK103 (imagen de referencia)
```

---

## 5. DESGLOSE POR MILESTONES (CALENDARIO DE PRODUCCIÓN)

### SEMANA 1: Blockout y Fundamentos

**Objetivo:** Primer nivel de prueba funcional, sin arte final. Solo geometría de blockout.

**Assets a crear:**
- [ ] Blockout del mapa industrial (geometría simple, sin texturas)
  - Usar cubos y planos en Blender, exportar como FBX estático
  - Dimensiones reales (escala correcta para el personaje)
  - Incluir: 3 edificios, 1 zona abierta, 2 puntos de extracción posibles
- [ ] Colección de piezas modulares industriales GREY BOX:
  - SM_Box_Wall_3m (muro recto)
  - SM_Box_Floor_1m (suelo)
  - SM_Box_Cover_Low (cobertura baja)
  - SM_Box_Cover_High (cobertura alta)
- [ ] Placeholder de personaje (cápsula con cabeza, sin animaciones reales)

**Entregables:**
- Archivo: `ENV_Industrial_Blockout_v01.blend`
- FBX exportados en carpeta `Exports/Week01/`
- Capturas del level en UE5 mostrando escala

---

### SEMANAS 2-3: Personaje Jugador Hero

**Objetivo:** Personaje jugador modelado, UV-ed, listo para texturizado. Sin rig aún.

**Assets a crear:**
- [ ] Modelo high-poly del personaje (para bake de normales)
  - Cabeza detallada (nariz, orejas, detalles faciales)
  - Cuerpo con ropa táctica: pantalón cargo, camiseta técnica, guantes
  - Equipo base: chaleco porta-cargadores (genérico sin marca)
- [ ] Modelo low-poly (dentro del presupuesto: 25K tris cuerpo + 8K cabeza)
  - Clean topology para deformación de rigs
  - Edge loops en articulaciones (codos, rodillas, hombros)
- [ ] UV Unwrap Channel 0 (textura)
  - Cabeza: 40% del espacio UV (más detail)
  - Cuerpo: 40%
  - Manos/pies: 20%
- [ ] UV Unwrap Channel 1 (lightmap, sin overlapping)

**Bake de normales:**
- High → Low poly normal bake en Blender (cycles)
- Exportar: T_Player_Skin_Normal.png, T_Player_Gear_Normal.png

**Entregables:**
- Archivo: `CHAR_Player_Base_v01.blend`
- Mesh exportado como FBX (sin rig aún)
- Normal maps baked en 2048×2048

---

### SEMANAS 2-4: AK-103 (Arma Hero)

**Objetivo:** AK-103 completamente modelado, UVs, textured, listo para rig de animación.

**Semana 2 — Modelado:**
- [ ] Modelo high-poly: cada parte del AK-103 por separado
  - Receptor (receiver), cañón, guardamanos, culata, cargador, mirilla
  - Referencia de foto: vista izquierda, derecha, superior, frente, trasera
- [ ] Modelo low-poly TPP: 8,000 tris total
- [ ] Modelo low-poly FPP: 15,000 tris (más detalle porque se ve de cerca)

**Semana 3 — UVs y Bake:**
- [ ] UV Channel 0 (textura) del modelo FPP y TPP
  - Aprovechar espacio: piezas metálicas con texel density alta (512px/m)
  - Piezas de polímero negro: pueden tener menos detail
- [ ] Bake de normales (High → Low)
- [ ] Bake de AO (Ambient Occlusion) para T_AK103_ORM

**Semana 4 — Texturas en Substance Painter:**
- [ ] Texturizado completo en Substance Painter
  - Smart Materials: Metal cepillado para receptor, polímero para guardamanos
  - Desgaste: arañazos en bordes metálicos, desgaste en empuñadura
  - Exportar: Albedo, Normal, ORM (2048×2048)
- [ ] Adjuntos del AK-103:
  - Cargador de 30 balas (genérico)
  - Cargador de 40 balas (variante)

**Entregables:**
- `WEAPON_AK103_v01.blend` (high-poly + low-poly + rig básico)
- FBX del TPP y FPP exportados
- Texturas 2048×2048: T_AK103_Albedo, T_AK103_Normal, T_AK103_ORM

---

### SEMANAS 3-5: Kit Modular Industrial (Arte Final)

**Objetivo:** Reemplazar el blockout con arte final. 30+ piezas modulares industriales.

**Lista completa de piezas industriales a modelar:**

#### Muros (6 variantes)
- [ ] SM_Ind_Wall_Straight_3m — Muro liso de concreto/ladrillo, 3m alto × 1m ancho
- [ ] SM_Ind_Wall_Corner_90 — Esquina a 90 grados
- [ ] SM_Ind_Wall_Window_A — Muro con ventana rectangular
- [ ] SM_Ind_Wall_Window_B — Muro con ventana rota (vidrios rotos)
- [ ] SM_Ind_Wall_DoorFrame — Marco de puerta sin puerta
- [ ] SM_Ind_Wall_Damaged — Muro dañado con boquetes

#### Suelos (3 variantes)
- [ ] SM_Ind_Floor_Concrete — Suelo de concreto limpio
- [ ] SM_Ind_Floor_MetalGrate — Rejilla metálica (semitransparente desde abajo)
- [ ] SM_Ind_Floor_Damaged — Concreto con grietas y boquetes

#### Techos
- [ ] SM_Ind_Ceiling_Flat — Techo liso
- [ ] SM_Ind_Ceiling_Beams — Techo con vigas de acero expuestas

#### Pilares y Estructuras
- [ ] SM_Ind_Pillar_Square_3m — Pilar cuadrado de 3m
- [ ] SM_Ind_Pillar_Round_3m — Pilar redondo de 3m (tubería industrial)
- [ ] SM_Ind_Beam_H_Horizontal — Viga H horizontal (soporte de techo)

#### Escaleras y Accesos
- [ ] SM_Ind_Stairs_Straight_1F — Escalera de un piso (3m alto)
- [ ] SM_Ind_Stairs_Metal_Grate — Escalera metálica de rejilla
- [ ] SM_Ind_Railing_Straight — Barandilla metálica (1m)

#### Props de Entorno
- [ ] SM_Ind_Crate_Wood_A — Caja de madera pequeña (50×50×50cm)
- [ ] SM_Ind_Crate_Wood_B — Caja de madera mediana (100×70×60cm)
- [ ] SM_Ind_Barrel_Metal — Bidón metálico (55 galones)
- [ ] SM_Ind_Barrel_Cluster — Grupo de 3 bidones
- [ ] SM_Ind_Shelf_Metal_Small — Estantería metálica pequeña
- [ ] SM_Ind_Shelf_Metal_Large — Estantería metálica grande (con objetos encima)
- [ ] SM_Ind_Pallet_Wood — Palet de madera
- [ ] SM_Ind_Tank_Industrial — Tanque industrial grande
- [ ] SM_Ind_Machinery_A — Máquina industrial genérica (forma interesante)
- [ ] SM_Ind_Machinery_B — Variante de maquinaria
- [ ] SM_Ind_ConveyorBelt — Cinta transportadora (no funcional, decorativa)
- [ ] SM_Ind_Pipe_Straight_A — Tubería horizontal
- [ ] SM_Ind_Pipe_Elbow_90 — Codo de tubería
- [ ] SM_Ind_FuseBox — Cuadro eléctrico (en muros)
- [ ] SM_Ind_Ladder_Metal — Escalera de mano en muro
- [ ] SM_Ind_Container_Shipping — Contenedor de transporte (exterior)

**Colisiones necesarias:** Crear UCX_ o UBX_ para cada pieza. Priorizar simplicidad.

---

### SEMANAS 4-6: Rig y Animaciones del Personaje

**Objetivo:** Personaje jugador con rig completo y animaciones de locomoción básicas.

**Semana 4 — Rig:**
- [ ] Armature con nomenclatura Epic Mannequin
- [ ] Weight painting completo (sin artefactos de deformación)
- [ ] Test de poses extremas (crouch, prone, salto, lean)
- [ ] Configurar IK constraints básicos

**Semana 5 — Animaciones base (tercera persona):**
- [ ] Idle parado (respiración sutil)
- [ ] Walk forward, backward, left, right
- [ ] Jog forward
- [ ] Sprint
- [ ] Crouch idle
- [ ] Crouch walk

**Semana 6 — Animaciones de combate:**
- [ ] Transición cadera → ADS
- [ ] Disparo (sacudida)
- [ ] Recarga estándar (con arma de referencia AK-103)
- [ ] Recarga táctica
- [ ] Sacar arma (equip)

---

### SEMANAS 5-7: Equipamiento y Adyacentes

**Objetivo:** Todo el equipo que el jugador puede vestir (cascos, chalecos, mochilas).

- [ ] SK_Helmet_Level2 — Casco militar nivel 2 (tipo PASGT)
- [ ] SK_Helmet_Level3 — Casco balístico nivel 3 (tipo Fast)
- [ ] SK_Armor_Level2 — Chaleco táctico nivel 2
- [ ] SK_Armor_Level3 — Armadura pecho nivel 3
- [ ] SK_Vest_Tactical — Chaleco porta-cargadores
- [ ] SK_Backpack_Small — Mochila pequeña
- [ ] SK_Backpack_Tactical — Mochila táctica mediana
- [ ] SK_Backpack_Large — Mochila grande

Todos deben:
- Estar modelados para attacharse al personaje sin clipping
- Tener sockets correctos para attachment en UE5
- Texturas 2048×2048 para tier 3+, 1024×1024 para tier 1-2

---

### SEMANAS 6-8: Enemigo PMC

**Objetivo:** Enemigo PMC Bot completamente funcional (modelo + rig + texturas + animaciones).

- [ ] Modelo high-poly PMC (similar a jugador pero variante visual clara)
- [ ] Modelo low-poly (18,000 tris base)
- [ ] Rig compatible con esqueleto del jugador (para reutilizar animaciones de locomoción)
- [ ] Diferencias visuales: uniforme diferente, equipo distinto, sin HUD
- [ ] Texturas: variante base, variante veteran (equipo más robusto)
- [ ] Animaciones específicas de IA: revisar esquinas, señalar objetivo, cubrir compañero

---

### SEMANAS 8-10: Mapa Industrial — Arte Final

**Objetivo:** Reemplazar el blockout del mapa industrial con arte final usando el kit modular.

- [ ] Construir toda la zona industrial con las piezas del kit
- [ ] Añadir props de entorno para dar vida al nivel
- [ ] Crear puntos de interés distintos visualmente
- [ ] Iluminación de test (luces de trabajo, no final)
- [ ] Vegetation/escombros en exteriores

---

### SEMANAS 10-12: Segunda Arma y Adjuntos

**Objetivo:** Segunda arma primaria y set completo de adjuntos básicos.

- [ ] M4A1 completo (modelo + UVs + texturas + rig)
- [ ] Adjuntos prioritarios con texturas finales:
  - Supresor 7.62 (para AK-103 y FAL)
  - Supresor 9mm (para MP5 y Glock)
  - Mira Red Dot Kobra
  - Mira Holográfica 552
  - ACOG TA31
  - Empuñadura vertical delantera
  - Freno de boca AK

---

## 6. RECURSOS DE REFERENCIA

### Herramientas Recomendadas

| Herramienta | Uso | Precio |
|-------------|-----|--------|
| Blender 4.x | Modelado, UV, rigging, animación | Gratuito |
| Substance Painter | Texturizado PBR | Suscripción/standalone |
| Substance Designer | Texturas proceduales tileables | Suscripción |
| Marmoset Toolbag 4 | Preview y bake de alta calidad | $99 licencia |
| PureRef | Organizar imágenes de referencia | Gratuito / donación |
| TexTools (Blender addon) | UV workflow avanzado | Gratuito |
| DragonBones / Mixamo | Referencia de poses/animaciones | Gratuito |
| UE5 Mannequin | Referencia de proporciones y rig | Incluido en UE5 |

### Referencias de Proporciones

Para escala correcta, usar siempre el Mannequin de UE5 como referencia:
- Importar el Mannequin al .blend de Blender como referencia visual
- El Mannequin mide exactamente 180cm — asegurarse que el personaje propio es similar

### Banco de Imágenes de Referencia

Organizar en PureRef por categoría:
- Referencias de armas reales (vistas ortogonales: izquierda, derecha, top, front)
- Referencias de equipo táctico (chalecos, cascos)
- Referencias de entornos industriales reales (plantas de fábrica, almacenes)
- Referencias de personajes (equipo militar/PMC real)
- Paletas de color del proyecto (consistencia visual)

---

## 7. CONTROL DE CALIDAD (QA CHECKS POR ASSET)

Antes de exportar cualquier asset como "terminado", verificar:

### Para Static Meshes
- [ ] Escala correcta (comparar con Mannequin de referencia)
- [ ] Pivot en posición correcta (esquina inferior izquierda para modulares)
- [ ] Sin normales invertidas (sin caras negras en Blender viewport)
- [ ] Sin vértices duplicados (Mesh → Merge by Distance)
- [ ] UV Channel 0: sin estiramientos mayores al 5%
- [ ] UV Channel 1: sin overlapping, margen de 2px entre islas (a resolución objetivo)
- [ ] Nombres de material slots corretos (convenio M_Tipo_Descripcion)
- [ ] LODs presentes y nombrados correctamente
- [ ] Colisiones UCX/UBX presentes y correctas
- [ ] FBX exportado con settings correctos (Forward: -Z, Up: Y)
- [ ] Verificado en UE5: escala, pivot, colisiones, materiales

### Para Skeletal Meshes
- [ ] Todo lo de Static Mesh, más:
- [ ] Armature con nombres Epic Mannequin
- [ ] Peso total de vértices = 1.0 en todos los vértices
- [ ] Máximo 4 influencias de hueso por vértice
- [ ] Rest pose en A-Pose
- [ ] Sin huesos "leaf bones" innecesarios en el export
- [ ] Test de deformación: mover huesos a extremos, verificar sin artefactos
- [ ] Socket de arma correctamente posicionado (si aplica)

### Para Texturas
- [ ] Resolución es potencia de 2 en ambas dimensiones
- [ ] Normal map en espacio de color Linear (Non-Color Data)
- [ ] ORM empaquetado correctamente (AO=R, Roughness=G, Metallic=B)
- [ ] Sin artefactos de compresión visibles en preview
- [ ] Suficiente detail para la distancia de visualización (texel density correcta)

---

*Este plan asume un equipo de 1-2 artistas 3D. Ajustar duraciones según el tamaño del equipo. Ver BLENDER_TO_UE5_PIPELINE.md para el pipeline técnico detallado.*
