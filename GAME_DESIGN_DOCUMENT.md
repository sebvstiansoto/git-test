# DOCUMENTO DE DISEÑO DE JUEGO (GDD)
## Proyecto: ZONA ROJA — Shooter de Extracción Táctico

**Versión:** 1.0  
**Fecha:** Junio 2026  
**Motor:** Unreal Engine 5  
**Género:** Shooter táctico de extracción PvPvE  
**Plataformas objetivo:** PC (Steam), consolas de próxima generación  

---

## 1. CONCEPTO DEL JUEGO

### 1.1 Visión General

ZONA ROJA es un shooter táctico de extracción en primera persona donde los jugadores asumen el rol de mercenarios (llamados "Operativos") que se infiltran en zonas de exclusión contaminadas para saquear recursos, completar misiones de alto riesgo y extraerse con vida. Cada sesión de juego es una incursión independiente donde la muerte permanente implica la pérdida del equipo llevado.

### 1.2 Pilares de Diseño

1. **Tensión sostenida:** Cada decisión tiene consecuencias reales. El equipo perdido duele.
2. **Profundidad táctica:** El conocimiento del mapa, el posicionamiento y la comunicación superan a los reflejos puros.
3. **Progresión significativa:** El jugador siempre avanza, incluso tras una muerte, gracias al sistema de stash y misiones.
4. **Autenticidad armamentística:** Las armas se comportan de forma realista con física de balística propia.
5. **Atmósfera opresiva:** El sonido, la iluminación y el diseño de nivel refuerzan la sensación de peligro constante.

### 1.3 Referentes

- **Escape from Tarkov:** Sistema de inventario, economía y progresión.
- **Hunt: Showdown:** Tensión PvPvE y diseño de mapa con objetivos claros.
- **The Cycle: Frontier:** Loop de extracción accesible sin perder profundidad.
- **ARMA 3:** Simulación de balística y comunicación táctica.

---

## 2. LOOP DE JUEGO PRINCIPAL

### 2.1 El Ciclo Insert → Saqueo → Extracción

```
[LOBBY/STASH]
     │
     ▼
[PREPARACIÓN]  ←── Equipar loadout desde stash
     │              Aceptar misiones activas
     │              Revisar mapa (info limitada)
     ▼
[INSERCIÓN]    ←── Caída en paracaídas o despliegue terrestre
     │              Tiempo de inserción: 60-90 segundos
     │              Posición aleatoria en zona de spawn
     ▼
[INCURSIÓN]    ←── Explorar POIs (Puntos de Interés)
     │              Saquear cuerpos, cajas, taquillas
     │              Eliminar IA enemiga (PMC, mutantes, torretas)
     │              Enfrentamientos PvP con otros Operativos
     │              Completar objetivos de misión
     ▼
[EXTRACCIÓN]   ←── Llegar a zona de extracción activa
     │              Tiempo de espera: 90 segundos en la zona
     │              Posible interrupción por otros jugadores
     ▼
[POST-RAID]    ←── Inventario saqueo → stash personal
                    Cálculo de XP y reputación
                    Venta de excedentes a traders
                    Desbloqueo de crafteo y misiones nuevas
```

### 2.2 Condiciones de Fin de Raid

| Condición | Resultado |
|-----------|-----------|
| Extracción exitosa | Conservas todo el loot + XP completa |
| Muerte en raid | Pierdes todo el equipo equipado (excepto seguro) |
| Tiempo agotado | Mueres, pierdes equipo (igual que muerte) |
| Desconexión | Personaje permanece 60s más, luego muere |

### 2.3 Duración de Sesión

- **Raid corta:** 15-25 minutos (mapas pequeños)
- **Raid estándar:** 25-45 minutos (mapas medianos)
- **Raid larga:** 45-75 minutos (mapas grandes, objetivos complejos)

---

## 3. MECÁNICAS DEL JUGADOR

### 3.1 Movimiento

#### Velocidades Base (en cm/s)
| Estado | Velocidad |
|--------|-----------|
| Caminar | 150 cm/s |
| Trotar | 300 cm/s |
| Correr (sprint) | 500 cm/s |
| Agachado | 120 cm/s |
| Prone (tumbado) | 60 cm/s |
| ADS caminando | 130 cm/s |

#### Estados de Movimiento
- **Caminar silencioso:** Tecla Ctrl mantenida. Sin ruido de pasos. Velocidad reducida 50%. No consume stamina.
- **Trotar:** Estado por defecto. Ruido moderado. Consume stamina a ritmo bajo.
- **Sprint:** Shift mantenido. Ruido alto. Consume stamina rápidamente. Arma baja (no puede disparar durante 0.5s al salir).
- **Agachado (crouch):** Tecla C. Reduce silueta. Mejora precisión. Velocidad reducida.
- **Prone:** Doble tap C. Silueta mínima. Máxima precisión estática. Levantarse tarda 1.2 segundos.
- **Lean izquierda/derecha:** Q/E. Permite asomarse con mínima exposición corporal.
- **Salto:** Barra espaciadora. Altura 120cm. No puede disparar durante salto.
- **Vault (trepar):** Automático al acercarse a obstáculos de altura correcta (50-120cm). Animación de 0.8s.

#### Sistema de Inercia
El personaje tiene inercia física. Cambiar dirección bruscamente tiene penalización de velocidad. El movimiento en superficies inclinadas afecta la velocidad y la estabilidad del arma.

### 3.2 Sistema de Stamina

```
Stamina Máxima: 100 puntos
Consumo Sprint: 15 puntos/segundo
Consumo Vault: 20 puntos
Regeneración en reposo: 20 puntos/segundo (tras 2s de pausa)
Regeneración caminando: 8 puntos/segundo

Efectos al quedar sin stamina:
- No puede correr
- Oscilación severa del arma (weapon sway +300%)
- Respiración audible (sonido que puede delatar posición)
- Recuperación completa tarda 5 segundos
```

#### Modificadores de Stamina por Peso
| Peso del equipo | Modificador Stamina |
|----------------|---------------------|
| < 10 kg | +20% stamina max, +15% regen |
| 10-20 kg | Sin modificador |
| 20-30 kg | -20% stamina max, -20% regen |
| 30-40 kg | -40% stamina max, -40% regen, -15% velocidad |
| > 40 kg | -60% stamina max, -50% regen, -30% velocidad |

### 3.3 Sistema de Salud

#### Zonas Corporales y HP
```
[CABEZA]        35 HP  — Hit mortal frecuente. Daño x2.5
  ├─ Casco (si equipado): absorbe primer impacto de proyectil nivel ≤ casco
[TÓRAX]         85 HP  — Zona central. Daño base.
[ABDOMEN]       70 HP  — Daño x0.8. Sangrado más probable.
[BRAZO DERECHO] 60 HP  — Daño reducido al arma (precisión -30% si < 25 HP)
[BRAZO IZQUIERDO]60 HP — Daño reducido. Afecta manejo de arma
[PIERNA DERECHA] 65 HP — Fractura posible. Cojera a <25 HP
[PIERNA IZQUIERDA]65 HP— Fractura posible. Cojera a <25 HP
```

#### Estados de Daño
- **Hemorragia:** Pierde 1 HP/2s. Cura con vendaje o kit médico.
- **Fractura:** Imposible correr. Cura con entablillado o Surgikit.
- **Conmoción:** Visión borrosa 3-8 segundos por impacto en cabeza.
- **Inconsciencia:** HP total < 10. Se cae al suelo. Necesita morfina o equipo médico.

#### Curación
| Ítem | Cura | Tiempo | Efecto secundario |
|------|------|--------|-------------------|
| Vendaje básico | Para sangrado | 4s | Ninguno |
| Botiquín small | +40 HP zona | 6s | Ninguno |
| Botiquín médico | +85 HP zona | 12s | Ninguno |
| Surgikit | Fractura + +60 HP | 15s | Temblor 10s |
| Morfina | Hemorragia+Fractura+Inconsciente | 3s | -15% stamina 60s |
| Adrenalina | +50 stamina, quita efectos | 2s | Temblor leve 30s |

### 3.4 Sistema de Inventario

#### Stash del Jugador
El stash es un grid 2D que el jugador expande con el tiempo o con dinero. Dimensiones iniciales: **10 x 20 celdas**.

#### Inventario en Raid
Estructura de cuadrícula portátil limitada por el tipo de mochila equipada:

| Mochila | Celdas | Peso máx |
|---------|--------|----------|
| Sin mochila | 4x4 | 5 kg |
| Mochila pequeña | 6x6 | 12 kg |
| Mochila táctica | 8x8 | 20 kg |
| Mochila grande | 10x10 | 30 kg |
| Mochila militar | 12x12 | 40 kg |

#### Slots de Equipamiento
```
ARMAS:
  Slot Primaria (arma larga)
  Slot Secundaria (pistola/SMG compacta)
  Slot Holster (pistola de respaldo, siempre accesible)

PROTECCIÓN:
  Slot Casco
  Slot Chaleco (protección balística)
  Slot Armadura de cuerpo

EQUIPAMIENTO:
  Slot Mochila
  Slot Rig/Chaleco porta-magazzines (acceso rápido 6-8 slots)
  Slot Gafas/Visor

BOLSILLOS (siempre disponibles, 2x2 cada uno):
  Bolsillo izquierdo
  Bolsillo derecho
```

---

## 4. SISTEMA DE ARMAS

### 4.1 Categorías de Armas

| Categoría | Ejemplos | Rol táctico |
|-----------|----------|-------------|
| Rifle de asalto | AK-103, M4A1, HK416 | Versátil, rango medio |
| Subfusil (SMG) | MP5, PP-19, Vector | CQC, silencioso |
| Rifle de batalla | FAL, M14, G3 | Largo alcance, penetración |
| Rifle de francotirador | SVD, M24, AWM | Muy largo alcance, 1-shot |
| Escopeta | Mossberg 590, SPAS-12 | CQC extremo, brechas |
| Ametralladora ligera (LMG) | PKM, M249 | Supresión, alto volumen |
| Pistola | Glock 17, 1911, Desert Eagle | Backup |
| Lanzagranadas | GP-25 (sub-arma) | Área, anti-blindaje ligero |

### 4.2 Estadísticas Base por Arma (ejemplo: AK-103)

```
Nombre:             AK-103
Calibre:            7.62x39mm
Daño base (torso):  58
Daño cabeza:        145 (x2.5)
Velocidad proyectil: 715 m/s
Cadencia de fuego:  600 RPM
Capacidad mag:      30 (estándar) / 10, 40, 75 (opcionales)
Recarga:            2.8s (normal) / 1.6s (táctica con mag disponible)
Distancia efectiva: 350m
Alcance máximo daño: 500m (falloff 100% a 700m)
Penetración:        38 (escala 1-100)
Dispersión base:    0.8 MOA
Retroceso vertical: 1.8 (escala arbitraria)
Retroceso horizontal: 0.6
Peso:               3.8 kg (sin cargador)
```

### 4.3 Sistema de Balística (ver WEAPONS_SYSTEM.md para detalle completo)

- Proyectiles con trayectoria física (caída por gravedad a distancias largas)
- Penetración de materiales: madera, drywall, metal delgado, vidrio, concreto
- Fragmentación en impacto dependiendo del ángulo y material
- Supresión auditiva (área de supresión psicológica por disparos cercanos)

### 4.4 Adjuntos (Sistema Modular)

**Slots disponibles por arma:**
- Riel Picatinny superior (óptica)
- Riel inferior (bípode, empuñadura táctica, linterna)
- Boca del cañón (silenciador, freno de boca, flash hider)
- Cañón (puede cambiarse en algunas armas)
- Culata (ergonomía, estabilidad)
- Revistero/Cargador (capacidad)
- Receptor/Grupo de gatillo (cadencia, seguro selectivo)

---

## 5. DISEÑO DE MAPAS

### 5.1 Principios de Diseño

1. **Lectura clara:** El jugador debe poder orientarse visualmente. Landmarks memorables.
2. **Múltiples rutas:** Siempre al menos 3 rutas entre puntos importantes (flanqueo).
3. **Zonas de riesgo/recompensa:** Las mejores recompensas en las zonas más expuestas.
4. **Funnel táctico:** Cuellos de botella naturales que generan conflicto, no obligatorio.
5. **Verticalidad:** Edificios de varios pisos, tejados accesibles, sótanos oscuros.

### 5.2 Zonas del Mapa

#### Zona de Alta Seguridad (Risk Tier 3)
- Mejor loot: armas de nivel medio-alto, equipo raro
- Alta densidad de enemigos IA (PMC bots, torretas activas)
- Múltiples accesos PvP
- Visibilidad limitada (niebla, humo industrial, escombros)

#### Zona Industrial (Risk Tier 2)
- Loot de nivel medio: materiales de crafteo, munición, consumibles
- Densidad media de enemigos
- Estructuras grandes, posiciones de francotirador
- Rutas de tránsito hacia Tier 3

#### Zona Residencial (Risk Tier 1)
- Loot básico: comida, vendas, objetos de bajo valor
- Pocos enemigos IA (patrullas sueltas)
- Zona de aterrizaje segura para jugadores sin equipo
- Puntos de extracción cercanos

### 5.3 Puntos de Interés (POIs)

Cada mapa debe tener entre 8-12 POIs con nombres propios y propósito claro:

| POI | Tipo | Loot primario | Riesgo |
|-----|------|---------------|--------|
| Base Militar Alpha | Edificio complejo | Armas, munición táctica | Muy alto |
| Almacén Central | Nave industrial | Equipamiento, mochilas | Alto |
| Hospital Abandonado | Edificio civil | Medicamentos, kits | Medio-alto |
| Centro Comercial | Complejo civil | Electrónica, consumibles | Medio |
| Comisaría | Edificio oficial | Armas de nivel medio | Medio |
| Refinería | Industrial exterior | Combustible, materiales | Alto |
| Torre de Comunicaciones | Estructura vertical | Electrónica rara | Medio |
| Barrio Obrero | Zona residencial | Comida, herramientas | Bajo |
| Estación de Tren | Hub de tránsito | Mixto | Variable |
| Bunker Subterráneo | Zona secreta | Loot premium, misiones | Extremo |

### 5.4 Zonas de Extracción

- Cada mapa tiene 4-6 puntos de extracción
- En cada raid, solo 2-3 están activos (seleccionados aleatoriamente al inicio)
- El jugador ve en el mapa qué extracciones están activas
- Tipos de extracción:
  - **Helipuerto:** Visible, radio 15m, tiempo espera 90s
  - **Túnel de escape:** Oculto, una sola persona a la vez, 30s
  - **Punto de pickup:** Requiere item especial (señal de humo)
  - **Vehículo blindado:** Disponible solo 5 minutos por raid, compite con otros jugadores

### 5.5 Zonas de Spawn

- Spawn aleatorio entre 4-8 puntos por borde del mapa
- Separación mínima garantizada entre spawns de distintos equipos
- Cooldown por zona (misma zona no se repite en dos raids seguidas para el mismo jugador)
- Zona segura de 60 segundos al inicio (sin IA activa en radio de 100m)

---

## 6. ENEMIGOS IA

### 6.1 Tipos de Enemigos

#### PMC Bot (Mercenario IA)
- Enemigo humanoide con equipo similar al jugador
- Usa coberturas, se flanquea, hace uso de granadas
- Tres niveles de dificultad: Novato, Veterano, Elite
- Suelto en grupos de 2-4

#### Guardia Militar
- Patrulla predefinida en instalaciones militares
- Mejor equipado que PMC bot en armamento
- Comunica posiciones al detectar al jugador

#### Mutante (infectado)
- Cuerpo a cuerpo únicamente
- Alta velocidad, baja salud
- Se mueve en hordas de 5-15
- Aparece en zonas contaminadas (rojo en mapa)

#### Centinela (torreta automática)
- Estática, activada por movimiento
- No dispara si el jugador se mueve muy despacio (sistema stealth)
- Puede desactivarse con hack electrónico (item especial)

#### Jefe de Zona (Boss)
- Un boss por mapa, spawn aleatorio entre 2 ubicaciones
- Gran cantidad de salud, equipo de alto nivel
- Loot único garantizado al eliminarlo
- Ejemplo: "Coronel Barkov" (armadura pesada, HK417 custom)

### 6.2 Sistema de Estados de Alerta

```
[PATROL] ──detección parcial──► [CURIOUS]
    ▲                               │
    │ pierde rastro (30s)           │ confirma amenaza
    │                               ▼
[RETURN]◄──no contacto 90s────[ALERT]
                                    │
                                    │ contacto visual/auditivo confirmado
                                    ▼
                               [COMBAT]
                                    │
                                    │ jugador muere / huye > 200m > 60s
                                    ▼
                              [SEARCHING]
                                    │
                                    │ sin contacto 120s
                                    ▼
                              [RETURN→PATROL]
```

#### Estado PATROL
- Sigue waypoints predefinidos o generados proceduralmente
- Gira la cabeza, hace pausas, revisa esquinas
- Radio de detección visual: 40m (cono 120°), auditivo: 20m radio
- De noche: visual reducido a 15m, auditivo sin cambio

#### Estado CURIOUS
- Se dirige al punto donde detectó algo
- Alerta a compañeros en radio de 30m (movimiento hacia punto)
- Velocidad aumentada 20%
- Radio visual aumenta a 60m

#### Estado ALERT
- Toma cobertura inmediata
- Avisa a todo el escuadrón (radio de comunicación: 80m)
- Solicita refuerzos si los hay disponibles
- Empieza a flanquear con 2+ miembros de escuadrón

#### Estado COMBAT
- IA agresiva con supresión y flanqueo coordinado
- Usa granadas si el jugador está en cobertura > 5 segundos
- Hace avances cortos, no permanece en el mismo sitio
- Intenta comunicar posición del jugador cada 15s (ruido que ayuda al jugador a trackear)

### 6.3 Comportamiento de Escuadrón

Los PMC bots operan en células de 2-4 miembros:

- **Líder de fuego:** Toma decisiones tácticas, flanquea
- **Tirador de apoyo:** Mantiene supresión desde cobertura
- **Explorador:** Se mueve adelante para detectar
- **Medic bot:** Cura compañeros caídos (sólo veteranos/elite)

Coordinación:
1. Si un miembro es abatido, los demás buscan cobertura y marcan posición
2. Si 2+ están en combate, el escuadrón intenta rodear al jugador
3. Retirada táctica si pierden a 3 de 4 miembros (buscan refuerzos)

---

## 7. ECONOMÍA DEL JUEGO

### 7.1 Monedas

| Moneda | Obtención | Uso |
|--------|-----------|-----|
| Créditos de Zona (CZ) | Vender loot, completar misiones | Comprar en traders, crafteo |
| Divisa Negra (DN) | Raids perfectas, boss kills, eventos | Comprar equipo premium, seguros |
| Tokens de Reputación | Progresión con cada trader | Desbloquear niveles de trader |

### 7.2 Sistema de Traders

#### Trader: KOVAL (Armas y munición)
- **Nivel 1 (0 rep):** Armas básicas, munición estándar
- **Nivel 2 (500 rep):** Armas de nivel medio, munición mejorada
- **Nivel 3 (2000 rep):** Armas militares, munición perforante
- **Nivel 4 (5000 rep):** Armas de alto rendimiento, munición especial

#### Trader: MÉDICA (Suministros médicos)
- Especializada en medicamentos y kits médicos
- Vende seguros de equipo (recuperar gear en caso de muerte)

#### Trader: TEKNIKA (Electrónica y equipo especial)
- Silenciadores, visores nocturnos, drones, equipos de hackeo
- Requiere misiones específicas para desbloquear items

#### Trader: BAZA (Materiales de crafteo)
- Materiales para crafteo en escondite
- Compra materiales industriales del jugador

### 7.3 Sistema de Seguros

El jugador puede asegurar su equipo antes de una raid:
- **Seguro básico (CZ):** Recupera el item si ningún jugador lo recoge en 24h reales
- **Seguro premium (DN):** Recupera el item siempre, en 12h reales
- Los items asegurados aparecen en el inventario de correo del trader tras el tiempo de espera

### 7.4 Tiers de Valor del Equipo

| Tier | Color | Valor CZ | Ejemplos |
|------|-------|----------|----------|
| 1 - Chatarra | Gris | 50-200 | Vendas usadas, comida barata |
| 2 - Común | Blanco | 200-500 | Munición básica, herramientas |
| 3 - Poco común | Verde | 500-2000 | Armas de nivel bajo, kits médicos |
| 4 - Raro | Azul | 2000-8000 | Armas militares, equipo táctico |
| 5 - Épico | Púrpura | 8000-25000 | Armas especiales, equipo de élite |
| 6 - Legendario | Naranja | 25000+ | Items únicos, equipo de boss |

### 7.5 Crafteo en Escondite (Hideout)

El jugador tiene una base personal (hideout) que puede mejorar:

| Módulo | Función | Materiales para construir |
|--------|---------|--------------------------|
| Zona médica | Craftea kits médicos | Metal, plástico, medicamentos |
| Taller de armas | Modifica y repara armas | Metal, tornillos, herramientas |
| Estación de recarga | Recarga munición personalizada | Latón, pólvora, proyectiles |
| Generador | Energía para módulos avanzados | Cable, baterías, combustible |
| Cámara de cultivo | Consumibles de stamina/salud | Tierra, semillas, agua |
| Centro de comunicaciones | Mapa mejorado, info de raids | Electrónica, cable, antena |

---

## 8. DISEÑO DE SONIDO

### 8.1 Principios Fundamentales

El sonido en ZONA ROJA es una mecánica de juego, no solo ambiental:

1. **Información táctica:** El jugador aprende la posición de enemigos por el sonido.
2. **Consecuencias del movimiento:** Los jugadores ajustan comportamiento según el ruido que hacen.
3. **Tensión atmosférica:** El silencio es tan importante como el ruido.
4. **Autenticidad:** Sonidos de armas reales, ambientes creíbles.

### 8.2 Capas de Audio

#### Capa 1: Ambiente Base (siempre activo)
- Viento, aves (más escasas en zonas peligrosas)
- Lluvia y clima dinámico (afecta alcance del sonido)
- Ruido industrial de fondo (instalaciones activas)
- Ruidos distantes de combate (otros jugadores/IA en el mapa)

#### Capa 2: IA Ambiental
- Conversaciones de patrullas a distancia
- Pisadas de enemigos (audibles a 15m)
- Radios de comunicación de guardias
- Gruñidos de mutantes (siempre en loop tenue en zona contaminada)

#### Capa 3: Jugador
- Respiración reactiva (ritmo cardíaco audible en situaciones de peligro)
- Pasos propios (volumen y tipo según superficie y velocidad)
- Equipamiento (tintineo de objetos en mochila, reducible con mochila silenciosa)
- Recarga y manejo de arma

#### Capa 4: Combate
- Disparos con modelo de oclusión 3D (HRTF)
- Eco según entorno (interior vs exterior)
- Supresión auditiva temporal al disparar sin protección
- Cracking sónico de balas al pasar cerca (efecto doppler)

### 8.3 Oclusión y Reverberación

- Motor de audio: MetaSounds en UE5 + middleware de oclusión (Resonance Audio o Steam Audio)
- Los disparos interiores suenan con reverb marcado, exterior con eco largo
- Las paredes reducen el volumen pero no el rango de detección del jugador (el jugador puede estimar desde dónde proviene)
- Materiales de obstrucción: madera (-6dB), ladrillo (-12dB), concreto (-18dB), metal (+2dB por resonancia)

### 8.4 Música

- Sin música durante la raid (solo efectos ambientales)
- Música de tensión sutil e intermitente al detectar peligro cercano (no intrusiva)
- Música de inserción al caer en paracaídas (épica, 30 segundos)
- Música de éxito al extraerse
- Música de derrota al morir

---

## 9. DISEÑO DE UI/HUD

### 9.1 Filosofía de HUD

HUD minimalista. La información crítica está visible; el resto requiere acción del jugador.

### 9.2 Elementos Permanentes en HUD

```
┌─────────────────────────────────────────────────┐
│ [MAPA mini]              [ESTADO MÉDICO]         │
│ (esquina inf-izq)        (esquina inf-der)       │
│                                                  │
│                    ●                             │
│                (retícula)                        │
│                                                  │
│ [ARMA actual]           [STAMINA bar]            │
│ [Mag: 28/90]            (bajo retícula)          │
│                                                  │
│                   [TIEMPO RAID: 00:23:45]        │
└─────────────────────────────────────────────────┘
```

- **Mapa mini:** Muestra al jugador como punto, extracciones activas como iconos, sin información de enemigos por defecto.
- **Estado médico:** Silueta humana simplificada con colores por zona. Verde=sano, Amarillo=daño, Rojo=crítico.
- **Info de arma:** Nombre del arma, munición en cargador / munición en reserva.
- **Stamina:** Barra que desaparece cuando está llena.
- **Tiempo de raid:** Tiempo restante de la raid (siempre visible).

### 9.3 Elementos Contextuales (aparecen según situación)

- **Indicador de sangrado:** Pulso rojo en bordes de pantalla.
- **Indicador de daño:** Flash de dirección al recibir impacto.
- **Indicador de escucha:** Vibraciones suaves en pantalla cuando hay ruido cercano (para accesibilidad).
- **Zona de extracción:** Indicador flotante cuando está cerca. "EXTRAER: 87s"
- **Mensaje de muerte:** "Muerto por [Nombre]" con zona corporal.
- **Notificación de misión:** Banner superior discreto.

### 9.4 Inventario (pantalla completa, fuera de combate ideal)

- Grid 2D para mochila e ítems.
- Arrastrar y soltar entre posiciones.
- Click derecho: menú contextual (usar, examinar, asegurar, tirar).
- Inspect mode: vista 3D del objeto en rotación.
- Peso total visible con barra de penalización.
- Tooltips detallados con estadísticas comparativas.

### 9.5 Mapa (pantalla completa)

- Disponible solo cuando el jugador está quieto o agachado (no en combate).
- Tiempo de apertura: 1.5 segundos (despliega mapa físico).
- Muestra: puntos de extracción activos, POIs, zona propia, peligros conocidos.
- No muestra: otros jugadores, posición de enemigos.
- El jugador puede añadir marcadores personales.

---

## 10. PROGRESIÓN DEL JUGADOR

### 10.1 Sistema de Niveles

- **Nivel de personaje (1-70):** Basado en XP de raids.
- Cada nivel desbloquea: nuevas misiones, acceso a traders de mayor nivel, slots adicionales de stash.

### 10.2 Fuentes de XP

| Actividad | XP |
|-----------|----|
| Extracción exitosa | 500 base |
| Sobrevivir raid completa | +100 por 10 minutos |
| Eliminar PMC bot | 50-150 (según nivel) |
| Eliminar jugador enemigo | 200-500 |
| Completar misión secundaria | 300-1000 |
| Descubrir nuevo POI | 100 |
| Headshot kill | +50 bonus |

### 10.3 Sistema de Habilidades Pasivas

A medida que el jugador realiza acciones específicas, mejoran habilidades pasivas:

| Habilidad | Acción que la mejora | Efecto máximo (nivel 10) |
|-----------|---------------------|--------------------------|
| Resistencia | Correr en raids | +30% stamina máxima |
| Sigilo | Caminar silenciosamente | -25% ruido de pasos |
| Medicina | Usar ítems médicos | -25% tiempo de curación |
| Puntería | Hacer kills con rifle | -15% dispersión |
| Fuerza | Llevar peso alto | +15 kg de carga |
| Percepción | Tiempo en zona caliente | +15% detección de loot raro |

---

*Fin del GDD v1.0 — Para detalles técnicos de implementación, ver UE5_ARCHITECTURE.md y WEAPONS_SYSTEM.md*
