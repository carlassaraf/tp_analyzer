# PCB del front end analógico

Estado actual de [analog.kicad_pcb](analog.kicad_pcb): ruteado completo y verificado. Este documento describe cómo está hecha la placa, por qué, cómo se verificó y qué falta antes de armarla.

Proceso de fabricación previsto:

- transferencia de tóner con impresora láser,
- grabado con cloruro férrico,
- doble faz **sin agujeros metalizados**: las vías se hacen a mano, con una pata de resistencia soldada de los dos lados,
- sin máscara antisoldante ni serigrafía.

Las coordenadas están en mm, en el sistema de coordenadas de KiCad, para ubicarlas en el editor.

## 1. Resumen

| Tema | Valor |
|---|---|
| Tamaño | 66 × 54 mm, con origen en (100, 50) |
| Cara inferior (B.Cu) | Canal de tensión (V) completo, divisor de red R5–R10 y bloque común (VBIAS y desacople): 42 SMD |
| Cara superior (F.Cu) | Canal de corriente (I) completo: 24 SMD |
| THT | J1, T1 y J2 montados arriba y soldados abajo. J10 montado abajo y soldado arriba |
| SMD | 0805, salvo R23 y R5–R10 en 1206 |
| Pistas | 0,4 mm con 0,3 mm de aislación; las de red, 0,6 mm |
| Vías | 38 de 1,3/0,7 mm (anillo de 0,3 mm) |
| Planos de GND | En las dos caras, fuera de la zona de red, unidos por 20 vías |
| Agujeros de montaje | 4 × M3 sin metalizar |
| Verificación | DRC con planos recalculados: 0 violaciones, 0 conexiones faltantes, 0 diferencias con el esquemático |

## 2. Zona de red (alta tensión)

### Qué es red en esta placa

| Nodo | Componentes |
|---|---|
| `Net-(J1-Pin_2)` | J1.2, R5.1 |
| `Net-(R5-Pad2)` … `Net-(R10-Pad1)` | Nodos intermedios del divisor R5–R10 |
| `Net-(R10-Pad2)` | R10.2, T1.1 (extremo "caliente" del primario) |
| `Net-(J1-Pin_1)` | J1.1, T1.2 |

Tensiones:

- Entre bornes de J1: hasta 380 V eficaces (537 V de pico) si se mide entre fases.
- Respecto de tierra: hasta 220 V eficaces (311 V de pico) en cualquier borne, más los transitorios de la red.

El GND de esta placa llega a la placa digital y de ahí al RP2350B y a lo que se le conecte (USB, PC). J10 y el cable del SCT-013 también son de baja tensión. Por eso entre la red y **cualquier** otro cobre hace falta aislación **reforzada**. La única barrera prevista es el ZMPT101B, y el PCB la respeta.

### Distancias

Valores orientativos, del orden de lo que piden IEC 61010-1 / IEC 60664-1 para 300 V, categoría II, grado de contaminación 2 y FR4. Conviene confirmarlos con la norma que use la cátedra. Como la placa no tiene máscara, se usa la misma cifra en aire y por superficie, en las dos caras.

| Entre | Distancia mínima |
|---|---|
| Red y cualquier cobre de baja tensión | 6 mm |
| Red y borde de placa | 3 mm |
| Red y agujeros de montaje | 7,5 mm (cubre la cabeza del tornillo M3) |
| Redes de red distintas entre sí | 1 mm |
| Bornes de J1 | 2,5 mm de cobre (pads de Ø 2,5 mm a 5 mm de paso) |
| Primario y secundario de T1 | 7,8 mm de pad a pad (lo da el ZMPT101B) |

### Cómo está hecha

- **Ubicación:** esquina superior izquierda. J1 está en el borde izquierdo. El divisor R5–R10 está en dos filas, en la cara inferior, debajo del lado del primario de T1. El secundario de T1 mira hacia el resto del circuito.
- **Ruteo:** las pistas de red están en la cara inferior, son de 0,6 mm y están **bloqueadas** en KiCad para no moverlas sin querer.
- **Planos:** los planos de GND no entran en la zona.
- **Guía visual:** el contorno de la zona está dibujado en la capa `User.Drawings`.
- **Reglas:** la clase de red `HV` agrupa las 8 redes de red, y [analog.kicad_dru](analog.kicad_dru) tiene las reglas de la tabla anterior, incluida la de superficie (`creepage`). El DRC controla todo esto: se probó que una pista de GND puesta a propósito a 2,7 mm de T1.1 dispara las reglas de aire y de superficie.

## 3. Distribución en dos caras

Cada canal se suelda del mismo lado que su conector, y así la mayoría de las conexiones no necesitan cambiar de cara:

- **Canal V en la cara inferior.** T1 y J2 se sueldan abajo, igual que los 24 SMD del canal.
- **Canal I en la cara superior.** J10 va montado del lado de abajo para que sus pads queden arriba, en la misma cara que el canal.
- **Bloque común en la cara inferior** (U1, R1–R4, C14, C15, C19, C21, C25, C26, C28). Queda del lado de J2, por donde entra la alimentación.

Ubicación de referencia:

| Bloque | Cara | Dónde |
|---|---|---|
| Zona de red: J1, R5–R10, primario de T1 | J1/T1 arriba, R5–R10 abajo | Esquina superior izquierda, hasta x ≈ 134 e y ≈ 75 |
| Canal V: U5 (148,4; 63,5), U7 (149,7; 80,3), burden R11 junto a T1.3 | B.Cu | Arriba a la derecha, entre T1 y J2 |
| Canal I: U4 (128,7; 91,9), U6 (140,7; 92,0), burden R23 junto a J10 | F.Cu | Abajo, a la derecha de J10 |
| Común: U1 (148,9; 98,3), C14 (155,7; 91,5) | B.Cu | Abajo a la derecha |
| J2 | Arriba, soldado abajo | Borde derecho, pin 1 en (159,3; 70,0) |
| J10 | Abajo, soldado arriba | Borde izquierdo, frente en la muesca del contorno |
| Agujeros M3 | — | (103,5; 80,0), (103,5; 100,5), (162,5; 53,5), (162,5; 100,5) |

## 4. Adaptaciones para fabricación casera

### Pads THT

Sin metalizado, cada pin THT solo conecta del lado donde se suelda:

- **J1, T1 y J2:** se sueldan abajo, así que sus pistas llegan solo por B.Cu.
- **J10:** se suelda arriba, así que sus pistas llegan solo por F.Cu.

KiCad 10 no permite sacar el anillo del lado del componente, así que queda como cobre aislado. Para que ese anillo no parezca conectado, los pads THT tienen la conexión a planos en "ninguna". Por eso cada pad de GND de J2 (2, 4, 6 y 8) tiene un tramo de pista que entra en el plano.

Los footprints adaptados están en la biblioteca del proyecto [tp_analog.pretty](tp_analog.pretty), registrada en [fp-lib-table](fp-lib-table):

| Componente | Pad / agujero | Cambio respecto del footprint estándar |
|---|---|---|
| J1 (bornera Altech AK100) | 2,5 / 1,3 mm | Pads más chicos, para tener 2,5 mm entre bornes |
| J2 (header IDC 2×7) | 2,0 / 1,0 mm | Pads más grandes, para agujerear a mano |
| J10 (jack CUI SJ1-3523N) | 2,4 / 1,4 mm | Agujeros redondos en lugar de ranuras de 0,4 mm, que no se pueden hacer a mano |
| T1 (ZMPT101B) | 2,2 / 1,2 mm | Sin cambios de tamaño, pads solo en la cara de soldadura |

### Vías

Las vías son de 1,3/0,7 mm. El anillo de 0,3 mm tolera el corrimiento de la mecha y el desalineado entre caras del tóner. Ninguna vía queda debajo del cuerpo de un componente de ninguna de las dos caras, porque la soldadura del alambre deja un bulto.

| Red | Vías |
|---|---|
| GND | 20 |
| +3,3 V, +5 V | 4 cada una |
| VADC, VPK | 2 cada una |
| VBIAS, IADC, IPK, SCK, MOSI, CS_I | 1 cada una |

### Planos de GND

- Hay un plano en cada cara (`GND_F` y `GND_B`), con 0,5 mm de aislación y 0,5 mm de ancho mínimo para que no queden astillas finas que se despeguen al grabar.
- Además, reducen mucho el cobre que tiene que atacar el cloruro férrico.

### Contorno

La muesca para el frente de J10 forma parte del contorno de la placa, que está cerrado y se controla en el DRC (`invalid_outline` como error).

## 5. Conector J2

El pinout coincide con el de los conectores J1–J4 de la [placa digital](../digital/README.md#j1j4-placas-analógicas-2x07-idc-cable-plano), con cada señal analógica entre dos GND del cable plano:

| Pin | Señal | Pin | Señal |
|---|---|---|---|
| 1 | VADC | 2 | GND |
| 3 | IADC | 4 | GND |
| 5 | VPK | 6 | GND |
| 7 | IPK | 8 | GND |
| 9 | SCK | 10 | MOSI |
| 11 | CS_V | 12 | CS_I |
| 13 | +3,3 V | 14 | +5 V |

El esquemático original tenía otro orden (por ejemplo, el pin 4 era CS_I en vez de GND) y se corrigió para que coincida con la placa digital, que no cambió.

Entre los pines de J2 no pasa ninguna pista: con pads de 2,0 mm a 2,54 mm de paso quedan 0,54 mm, y una pista con aislación necesita 1,0 mm.

## 6. Verificación

Además del DRC de KiCad (`kicad-cli pcb drc --schematic-parity --refill-zones --severity-all`), se revisó con un script lo que el DRC no ve en una placa sin metalizar:

| Chequeo | Resultado |
|---|---|
| DRC con planos recalculados y paridad con el esquemático | 0 violaciones, 0 conexiones faltantes, 0 diferencias |
| Pistas que llegan a un pad THT por la cara del componente (ese anillo no se suelda) | Ninguna |
| Vías debajo del cuerpo de un componente | Ninguna |
| Separación entre vías | Mínimo 1,6 mm entre centros |
| SCK, MOSI y CS cerca de nodos sensibles (< 1,5 mm en la misma cara, < 0,8 mm en la opuesta) | Ninguno |
| Desacople de U4–U7 (C27, C29, C33, C38) | A 2–4 mm del pin de alimentación |
| Zona de red | Sin violaciones de las reglas de 6 mm |

Nodos sensibles considerados: entradas de U4A y U5A, nodos de los capacitores de pico C34–C37 (C34 y C36 tienen 1 MΩ a VBIAS) y las entradas de los sensores (`vt` y `ct`).

**Vías que tocan pads.** En 11 vías el anillo toca el borde de un pad SMD de su misma red, aunque el agujero queda afuera:

- **GND:** C14.2, C19.2, D5.1, D8.1, U6.4, y la que comparten R13.2 y C38.2.
- **VBIAS:** C37.2.
- **+5 V:** C33.1.
- **+3,3 V:** C29.1.
- **VADC:** R18.2.

Eléctricamente está bien; hay que tenerlo en cuenta al armar (sección 8).

**Menor.** Hay 28 tramos de pista de menos de 0,05 mm, restos del ruteo. No afectan; se limpian con *Herramientas → Limpiar pistas y vías*.

## 7. Reglas del proyecto

- **[analog.kicad_pro](analog.kicad_pro):**
  - Mínimos: aislación 0,3 mm, pista 0,3 mm, vía 1,3 mm con anillo de 0,3 mm, agujero 0,6 mm, agujero a cobre 0,3 mm, cobre al borde 0,5 mm.
  - Clase Default: aislación 0,3 mm, pista 0,4 mm, vía 1,3/0,7 mm.
  - Clase HV: aislación 1,0 mm y pista 0,6 mm, asignada por patrones a las 8 redes de red.
  - `courtyards_overlap`, `missing_courtyard` y `starved_thermal` como aviso; `invalid_outline` como error.
- **[analog.kicad_dru](analog.kicad_dru):**
  - HV a baja tensión: 6 mm en aire (`clearance`) y por superficie (`creepage`).
  - HV al borde: 3 mm.
  - HV a agujeros de otras redes: 6 mm.
  - HV a agujeros de montaje: 7,5 mm.
  - Entre bornes de J1: 1 mm.

Si se modifica el ruteo:

- Respetar las reglas de la sección 4: pistas de THT solo por la cara de soldadura, un tramo de pista para conectar un pad THT al plano y ninguna vía debajo de un componente.
- Correr el DRC con `--refill-zones`.

## 8. Antes de armar

1. **Verificar footprints con las piezas reales:**
   - Que la pata del jack J10 entre en el agujero de 1,4 mm.
   - La separación de pines del ZMPT101B.
   - Que la bornera J1 esté especificada para 400 V o más. Muchas borneras de 5 mm son de 250–300 V; si no alcanza, usar una de 7,5 mm de paso.
2. **Mecánica de J10:** el cuerpo del jack queda debajo de la placa. Los separadores del gabinete tienen que ser más altos que el jack (ver la altura del SJ1-3523N en la hoja de datos), y la abertura del panel tiene que quedar del lado de abajo.
3. **Tornillos:** usar tornillos y separadores de nylon, o al menos verificar que los metálicos no se acerquen a la zona de red.
4. **Imprimir para el tóner:** la cara superior espejada y la inferior sin espejar. Usar los 4 agujeros de montaje como guía de alineación entre caras.
5. **Orden de armado:**
   1. Vías, cortadas al ras. Primero las que tocan pads (sección 6), porque el estaño del pad se une con el de la vía.
   2. SMD de la cara inferior (canal V, divisor y bloque común).
   3. SMD de la cara superior (canal I).
   4. THT al final: T1, J1 y J2 desde arriba, soldados abajo; J10 desde abajo, soldado arriba.
6. **Después de soldar:** limpiar el flux y barnizar la zona de red (laca acrílica o barniz para placas), porque la placa no tiene máscara.
7. **Primeras pruebas con red:** alimentar la placa digital desde una notebook a batería o con un aislador USB, hasta haber verificado la aislación.
