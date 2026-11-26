# PCB de la placa digital

Estado de [digital.kicad_pcb](digital.kicad_pcb): **ubicada y ruteada**. DRC con paridad de esquemático y todas las severidades activas: 0 violaciones, 0 conexiones pendientes, 0 errores de footprint.

Proceso, el mismo de la [placa analógica](../analog/revision_pcb.md):

- transferencia de tóner y cloruro férrico,
- doble faz sin agujeros metalizados,
- vías a mano con una pata de resistencia,
- sin máscara.

Coordenadas en mm, en el sistema de KiCad.

## 1. Resumen

| Tema | Valor |
|---|---|
| Tamaño | 65 × 76 mm, de (100, 50) a (165, 126) |
| Componentes | Todos los SMD en B.Cu. Los THT (J1–J6) montados arriba y soldados abajo: se suelda todo del mismo lado |
| B.Cu | Todas las señales |
| F.Cu | Troncos de 5 V y 3,3 V, 8 puentes de señal y plano de GND |
| Planos | GND en las dos caras |
| Vías | 40 (sección 5) |
| Pistas de señal | 0,4 mm. 0,3 mm en los puentes bajo capacitores y entre las filas de U2 |
| Pistas de alimentación | 0,8 mm en los troncos de F.Cu. 0,4–0,6 mm en las bajadas, que llevan poca corriente: ~4 mA en total en 3,3 V y ~11 mA por conector en 5 V |
| Agujeros | 4 × M3 sin metalizar: H1 (104; 108), H2 (160,6; 122,2), H3 (104; 122,2), H4 (104; 98,6) |

## 2. Ubicación

| Bloque | Componentes | Dónde |
|---|---|---|
| Conectores de línea | J1 (L1), J2 (L2), J3 (L3), J4 (N) | Fila arriba. Pin 1 en y = 55,8 y x = 118,5 / 128,7 / 138,9 / 149,1 |
| RP2350B | J5 (2×13) | Borde izquierdo, pin 1 en (106,4; 57,1). La fila impar mira al borde |
| Expansión | J6 (1×08) | Borde inferior, pin 1 en (152,3; 123,6) |
| ADC de L1 y L2 | U2, C8–C10 | U2 en (119,4; 106). Desacople debajo, entre los pines 12–14 |
| Expansor | U1, C3 | (139,4; 110,5). GPA (pines 21–28) mira a los conectores. C3 debajo de VDD/VSS |
| ADC de L3 y N | U3, C15–C17 | U3 en (158,4; 103). Desacople debajo, entre los pines 12–14 |
| Entradas de los ADC | C4, C11 (L1), C12, C5 (L2), C13, C6 (L3), C14, C7 (N) | Sobre las pistas de picos, cerca de cada ADC |
| Pull-ups de CS | R12–R19 | Debajo de cada conector, a los dos lados de las vías de alimentación |
| 33 Ω | R1–R8 | R1, R3 y R5 junto a la salida de SCK de su conector. R6 entre J3 y J4, más abajo. R7, R8, R4 y R2 en fila, debajo de J4 y hacia el borde derecho |
| Varios | R9 (RESET), R10, R11, R20, R21 | R9 entre U2 y U1. R10, R11 y R20 sobre el I2C, debajo de U2/U1. R21 junto a U3 |
| Reserva | C1 (3,3 V), C2 (5 V) | C1 junto a J5. C2 al final del tronco de 5 V |

## 3. Cómo está ruteado

El pinout del 2×7 lo fija la placa analógica, y un THT montado arriba no se puede espejar: los cuatro conectores tienen el mismo orden en el mismo sentido. Casi todas las decisiones salen de ahí.

### 3.1 Haz analógico

VADC e IADC salen de J5 (pines 1–15), suben y corren por el borde superior, por encima de los conectores. Entran a los pines 1 y 3 de cada conector desde arriba. No llevan vías.

### 3.2 Salida de cada conector

- **Izquierda:** VPK, IPK, SCK y CS_V salen hacia la izquierda y bajan.
- **Derecha:** CS_I, MOSI y GND salen hacia la derecha y bajan.
- **Alimentación:** los pines 13 (3,3 V) y 14 (5 V) bajan derecho a una vía cada uno. Están rodeados por las salidas de su propio conector, así que no hay otra forma de alimentarlos.
- **Pull-ups de CS:** las dos de cada conector quedan entre CS_V y CS_I. Toman el 3,3 V de la misma vía del pin 13.

### 3.3 Alimentación

- **5 V, por F.Cu:** sube desde J5 por el borde izquierdo, pasa por encima de J5 y corre en y = 75,6, justo debajo de los conectores. Termina en C2.
- **3,3 V, por F.Cu:** desde J5 pasa por debajo de J5 y corre en y = 79,6.
- **Tronco sur de 3,3 V:** baja por x = 111 y corre en y = 121. Alimenta con una vía cada uno a cuatro grupos:
  - U2 y su desacople,
  - R9,
  - U3 y su desacople,
  - U1, C3, J6 y las pull-ups del I2C y de los CS de los ADC.

### 3.4 Chip selects

- **L2:** va directo por B.Cu a los pines 23 y 24 de U1. Pasa por debajo de R3.
- **L1, L3 y N:** cada CS baja de su pull-up a una vía, cruza por F.Cu y vuelve a B.Cu con otra vía encima de U1. Son 12 vías.
- **Por qué hay saltos:** los picos de L1 y L2 van a U2, a la izquierda de U1, y los de L3 y N van a U3, a la derecha. En el camino cruzan los CS de las líneas vecinas. Con este orden de pines, seis de las dieciséis pistas (picos y CS) tienen que saltar sí o sí. Se eligió saltar CS porque los picos llevan los capacitores de entrada.

### 3.5 Picos

- **L1:** VPK e IPK pasan por debajo de R1 y bajan a los pines 1 y 2 de U2. IPK pasa entre los pads de C4.
- **L2:** VPK e IPK bajan en diagonal a los pines 3 y 4 de U2. IPK pasa entre los pads de C12.
- **L3:** VPK e IPK corren en horizontal por debajo de R5.
- **N:** VPK e IPK pasan por debajo de R6 y se suman por encima de L3.
- **Llegada a U3:** las cuatro bajan juntas a los pines 1–4. VPK_L3 pasa entre los pads de C13 e IPK_N entre los de C7.
- **GND de los capacitores:** C4 y C11 toman GND de J1. C12 y C5 lo toman de U2. C6, C7, C13 y C14 lo toman de U3.

### 3.6 SPI

- **Bus principal:** sale de J5 (pines 18, 20 y 22) y baja por la izquierda. Pasa por debajo de U2 y entra a los pines 11, 10 y 9 desde afuera. Sigue a través de los pads hacia el espacio entre las filas de U2. De ahí pasa entre las filas de U1 y llega desde abajo a los pines 11, 10 y 9 de U3.
- **SCK hacia R5 y R3:** una rama sale del extremo derecho del espacio entre las filas de U1 y sube. Después corre hacia la izquierda en y = 94,6, entre los picos de L3 y los CS de L2.
- **R2, R4, R6, R7 y R8:** SCK y MOSI salen de U3 por el lado interior de los pines 11 y 9. Pasan entre las filas de U3 y suben por el borde derecho.
  - MOSI pasa entre los pads de R7 y toca R2, R4, R8 y R6.
  - SCK pasa debajo de R2, R4 y R8 y llega a R7.
- **MOSI_L1 y MOSI_L2:** cruzan por F.Cu hasta R2 y R4, por encima de las vías de los CS. Son 2 vías cada una.
- **MISO:** solo une U2, U3 y J5.

### 3.7 I2C, CS de los ADC, RESET y J6

- **SDA y SCL:** bajan por la izquierda por fuera del bus, pasan por debajo y llegan desde abajo a los pines 13 y 12 de U1.
  - R11 está sobre SCL.
  - R10 tiene SCL pasando entre sus pads.
  - R20 tiene SDA y SCL pasando entre sus pads.
- **CS_ADC_L12:** va del pin 1 de U1 al pin 8 de U2 por el espacio entre las filas de U1.
- **CS_ADC_L3N:** va del pin 2 de U1, por debajo del bus, al pin 8 de U3. R21 queda al costado.
- **RESET:** sube derecho a R9.
- **J6:** escalera desde los pines 3–8 de U1.

### 3.8 GND

- **Planos:** las dos caras tienen plano de GND.
- **Islas:** las señales dividen el GND de B.Cu en nueve islas. Cada una tiene una vía al plano de F.Cu.
- **Vías ahorradas:** dos islas se unieron con pistas entre las filas de los ADC.
  - En U2, el pin 7 se une al pin 12 con una pista de 0,3 mm.
  - En U3, el pin 7 se une al pin 12 de la misma forma, con una pista de 0,4 mm.

## 4. Puentes

En una placa sin máscara, "puente" quiere decir una pista que pasa entre los dos pads de un componente, por debajo del cuerpo.

| Componente | Pasa por debajo | Ancho |
|---|---|---|
| R1 | VPK_L1, IPK_L1 | 0,4 mm |
| R3 | CS_V_L2, CS_I_L2 | 0,4 mm |
| R5 | VPK_L3, IPK_L3 | 0,4 mm |
| R6 | VPK_N, IPK_N | 0,4 mm |
| R7 | MOSI | 0,4 mm |
| R10 | SCL | 0,4 mm |
| R20 | SDA, SCL | 0,4 mm |
| C4 | IPK_L1 | 0,3 mm |
| C12 | IPK_L2 | 0,3 mm |
| C13 | VPK_L3 | 0,3 mm |
| C7 | IPK_N | 0,3 mm |

Entre las filas de los integrados también pasan pistas:

| Integrado | Pistas |
|---|---|
| U2 | SCK, MISO, MOSI y GND, a 0,3 mm |
| U1 | SCK, MISO, MOSI y CS_ADC_L12 |
| U3 | GND, SCK y MOSI |

Al soldar:

- **Antes de soldar los componentes de la tabla:** cubrir la pista de abajo con una capa fina de barniz o esmalte, o con un trozo de kapton.
- **Al soldar:** usar poco estaño.
- **Después:** medir que ningún pad haga contacto con la pista que pasa por debajo.
- **Tamaños:** las resistencias tienen que ser 1206 y los capacitores 0805. Un tamaño menor no deja lugar a las pistas.

## 5. Vías

Son 40. Ninguna queda dentro de un courtyard: la regla del [.kicad_dru](digital.kicad_dru) lo prohíbe, para que el bulto de la vía no levante ningún componente.

| Uso | Vías | Detalle |
|---|---|---|
| 5 V | 6 | J5, una por conector (pin 14), C2 |
| 3,3 V | 9 | J5, una por conector (pin 13 y sus dos pull-ups), U2, R9, U3, y U1 con J6 |
| GND | 9 | Una por isla (ver abajo) |
| Puentes de CS | 12 | CS_V y CS_I de L1, L3 y N, dos vías cada uno |
| MOSI_L1 y MOSI_L2 | 4 | Cruce hasta R2 y R4 |

Las nueve islas de GND, cada una con su vía:

1. J1, con C4 y C11.
2. J2.
3. J3.
4. J4, con C2.
5. J5, pines 2–16.
6. J5 pin 25, con C1.
7. U2 y A0–A2 de U1, con C5, C8–C10 y C12.
8. U3, con C6, C7 y C13–C17.
9. VSS de U1, con C3 y el pin 8 de J6.

Cada vía de esta lista se debe a una zona que las pistas de señal encierran del todo. Para bajar la cantidad habría que cambiar algún pinout: el 2×7 de las placas analógicas o el orden de GPA en U1.

## 6. Reglas

[digital.kicad_pro](digital.kicad_pro), las mismas mínimas que la placa analógica:

| Parámetro | Valor |
|---|---|
| Aislación | 0,3 mm |
| Ancho de pista mínimo | 0,3 mm |
| Vía | 1,3 mm, agujero de 0,7 mm |
| Cobre al borde | 0,5 mm |
| Clase `Default` | Pista de 0,4 mm |
| Clase `PWR` (+3.3V y +5V) | Pista de 0,8 mm |

- **[digital.kicad_dru](digital.kicad_dru):** prohíbe vías dentro de cualquier courtyard y pide 1,2 mm de cobre a los agujeros M3.
- **Planos de GND:** 0,4 mm de aislación (un poco más que el mínimo, para que el cloruro no deje puentes), ancho mínimo de 0,3 mm y alivio térmico en los pads SMD.
- **Pads THT:** conexión a planos en "ninguna". El anillo de arriba no se suelda, así que un THT solo se conecta por pista en B.Cu.

## 7. Cambios en el esquemático para esta placa

Son intercambios de pines y cambios de footprint; no cambia ningún componente.

| Qué | Antes | Ahora | Por qué |
|---|---|---|---|
| J5, pines 1–15 | VADC en el impar más bajo de cada par | IADC en el impar más bajo | Orden en que llega el haz analógico |
| J5, pines 17–26 | 17 SCK, 18 MOSI, 19 MISO, 20 GND, 21 SDA, 22 SCL, 23/24 +3,3 V, 25/26 +5 V | 17/19 +5 V, 21/23 +3,3 V, 25 GND; 18 SCK, 20 MISO, 22 MOSI, 24 SDA, 26 SCL | La alimentación va del lado del borde, a una vía. Las señales salen del lado interior en el orden en que llegan a U2 y U1 |
| U2 | VPK de las 4 líneas | CH0–3 = VPK_L1, IPK_L1, VPK_L2, IPK_L2 | Cada ADC lee dos líneas vecinas |
| U3 | IPK de las 4 líneas | CH0–3 = VPK_L3, IPK_L3, VPK_N, IPK_N | Ídem |
| CS de los ADC | CS_ADC_V, CS_ADC_I | CS_ADC_L12, CS_ADC_L3N (GPB0, GPB1) | Nombre acorde a la nueva función |
| U1, GPA0–7 | CS_V de L1..N, después CS_I | V/I de L1, V/I de L2, V/I de L3, V/I de N | Mismo orden en que salen de los conectores |
| R1–R21 | 0805 | 1206 | Lugar para las pistas que pasan por debajo |
| Footprints J1–J6 | Estándar | [tp_digital.pretty](tp_digital.pretty): pads de 2,0 mm, agujero de 1,0 mm | Agujerear a mano |

El firmware y el cable hacia el devboard tienen que seguir las tablas del [README](README.md). ERC: 0 errores; la única advertencia es la de los dos Dout que comparten MISO, que es intencional.

## 8. Pendientes

1. Verificar del lado del cable hacia qué lado tiene que quedar la muesca de J1–J5, para que los cables planos no se crucen.
2. Armar el cable o adaptador de J5 hacia el devboard con el pinout nuevo.
3. Serigrafía: no está acomodada, y el proyecto no la controla en el DRC. Puede haber referencias encimadas. Para la transferencia de tóner no importa. Si se imprime una guía de montaje, usar la vista de KiCad.
4. Después de cualquier retoque, correr:

   ```
   kicad-cli pcb drc --schematic-parity --refill-zones --severity-all digital.kicad_pcb
   ```
