# Placa digital: ADC de picos y expansor de E/S

Esta placa es el punto de unión entre las 4 placas analógicas (una por línea: L1, L2, L3 y N) y la placa del RP2350B. Todas las señales de las placas analógicas pasan por acá, así que hacia el RP2350B sale **un solo conector**.

- Esquemático: [digital.kicad_sch](digital.kicad_sch) (una hoja, A3).
- Placa analógica (una por línea): [../analog](../analog).

## 1. Qué hace

| Función | Cómo |
|---|---|
| Leer los detectores de pico (VPK, IPK) de las 4 líneas | Dos MCP3204 (12 bits, 4 canales, SPI): U2 para L1 y L2, U3 para L3 y N |
| Generar los 10 chip selects (8 MCP41010 + 2 MCP3204) sin usar pines del RP2350B | Expansor MCP23017 por I2C (U1) |
| Llevar las 8 señales del canal principal (VADC, IADC) al ADC del RP2350B | Pasan directo de J1–J4 a J5 |
| Repartir alimentación y SPI a las placas analógicas | J5 → J1–J4 |

```
 Placa analógica L1 ──┐ 2x07                                       2x13
 Placa analógica L2 ──┤   VADC, IADC ─────────────────────────────►┐
 Placa analógica L3 ──┤   VPK, IPK L1/L2 ──► U2 ──┐               │
 Placa analógica N  ──┘   VPK, IPK L3/N  ──► U3 ──┤ MISO          ├── J5 ──► RP2350B
              ▲  SCK/MOSI (33 Ω por línea) ◄────┤ SCK, MOSI        │         (ADC GPIO40-47,
              └─ CS_V, CS_I ◄── U1 MCP23017 ◄───┘ SDA, SCL ────────┘          SPI, I2C, 5 V, 3,3 V)
```

## 2. Conectores

### J1–J4: placas analógicas (2x07, IDC, cable plano)

Mismo pinout en la placa analógica. Cada señal analógica viaja entre dos GND del cable plano.

| Pin | Señal | Pin | Señal |
|---|---|---|---|
| 1 | VADC (canal principal de tensión) | 2 | GND |
| 3 | IADC (canal principal de corriente) | 4 | GND |
| 5 | VPK (detector de pico de tensión) | 6 | GND |
| 7 | IPK (detector de pico de corriente) | 8 | GND |
| 9 | SCK | 10 | MOSI |
| 11 | CS_V (MCP41010 de tensión) | 12 | CS_I (MCP41010 de corriente) |
| 13 | +3,3 V | 14 | +5 V |

J1 = L1, J2 = L2, J3 = L3, J4 = N. En el esquemático las redes llevan el sufijo de la línea: `VADC_L1`, `CS_I_N`, etc.

### J5: RP2350B (2x13, IDC)

| Pin | Señal | Pin | Señal |
|---|---|---|---|
| 1 | IADC_L1 | 2 | GND |
| 3 | VADC_L1 | 4 | GND |
| 5 | IADC_L2 | 6 | GND |
| 7 | VADC_L2 | 8 | GND |
| 9 | IADC_L3 | 10 | GND |
| 11 | VADC_L3 | 12 | GND |
| 13 | IADC_N | 14 | GND |
| 15 | VADC_N | 16 | GND |
| 17 | +5 V | 18 | SCK |
| 19 | +5 V | 20 | MISO |
| 21 | +3,3 V | 22 | MOSI |
| 23 | +3,3 V | 24 | SDA |
| 25 | GND | 26 | SCL |

Los pines 1–15 van a las entradas de ADC del RP2350B (GPIO40–47). Dentro de cada par, IADC va en el pin impar más bajo y VADC en el siguiente: es el orden en que llegan las pistas por el borde de la placa (ver [placa_pcb.md](placa_pcb.md)). El cable o adaptador hacia el devboard tiene que respetar esta tabla. La alimentación de todo el sistema analógico entra por J5.

Pines 17–26: la fila impar mira al borde de la placa y lleva solo alimentación y GND, que pasan por vía al lado del conector. La fila par mira al interior y lleva las cinco señales digitales, en el orden en que llegan a U2 y U1 sin cruzarse. En el cable plano cada señal digital queda entre dos hilos de alimentación o GND.

### J6: expansión (1x08)

GPB2–GPB7 del MCP23017 (libres), +3,3 V y GND.

## 3. Chip selects (MCP23017, dirección 0x20)

A2, A1 y A0 van a GND: dirección I2C **0x20**. RESET con pull-up de 10 kΩ (R9). INTA e INTB sin conectar.

| Pin del MCP23017 | Señal | Dispositivo |
|---|---|---|
| GPA0 | CS_V_L1 | MCP41010, canal de tensión L1 |
| GPA1 | CS_I_L1 | MCP41010, canal de corriente L1 |
| GPA2 | CS_V_L2 | MCP41010, canal de tensión L2 |
| GPA3 | CS_I_L2 | MCP41010, canal de corriente L2 |
| GPA4 | CS_V_L3 | MCP41010, canal de tensión L3 |
| GPA5 | CS_I_L3 | MCP41010, canal de corriente L3 |
| GPA6 | CS_V_N | MCP41010, canal de tensión N |
| GPA7 | CS_I_N | MCP41010, canal de corriente N |
| GPB0 | CS_ADC_L12 | U2, MCP3204 de picos de L1 y L2 |
| GPB1 | CS_ADC_L3N | U3, MCP3204 de picos de L3 y N |
| GPB2–GPB7 | — | Libres, en J6 |

- **Pull-ups de 10 kΩ en los 10 CS (R12–R21).** Después del encendido o de un reset, los pines del MCP23017 son entradas (alta impedancia). Sin pull-up, un CS podría quedar flotando en bajo y un MCP41010 tomaría basura del bus SPI como un cambio de ganancia. Con los pull-ups todo queda deseleccionado hasta que el firmware configura el expansor.
- **GPA7 y GPB7 son solo salida** en la hoja de datos actual del MCP23017. Acá se usan como salidas, así que no hay problema.

## 4. ADC de picos (MCP3204)

Cada MCP3204 lee las dos líneas que tiene al lado en la placa (U2: L1 y L2, U3: L3 y N), en lugar de separar tensión y corriente. Así los picos de cada par de conectores llegan juntos al ADC y se ahorran cruces en el ruteo (ver [placa_pcb.md](placa_pcb.md)). Para el firmware solo cambia el mapa de canales.

| ADC | CH0 | CH1 | CH2 | CH3 |
|---|---|---|---|---|
| U2 (L1, L2) | VPK_L1 | IPK_L1 | VPK_L2 | IPK_L2 |
| U3 (L3, N) | VPK_L3 | IPK_L3 | VPK_N | IPK_N |

- **VREF = 3,3 V, ratiométrico.** Las placas analógicas toman su 3,3 V de esta placa, y su VBIAS es 3,3 V / 2. Si el 3,3 V se mueve, VBIAS y la referencia se mueven juntos, y VBIAS sigue cayendo en ~2048 cuentas. El offset de cada canal (tolerancia del divisor de VBIAS y fuga del detector) se calibra midiendo sin señal.
- Rango: el detector entrega VBIAS + 0 a 1,3 V, o sea 1,65 a 2,95 V, dentro de 0–3,3 V. 1 LSB = 0,81 mV.
- **100 nF en cada entrada (C4–C7, C11–C14).** El capacitor de muestreo del MCP3204 (~20 pF) se carga de golpe al empezar la conversión; los 100 nF le dan esa carga sin que la tensión se mueva (20 pF / 100 nF → < 1 LSB). Con los 10 kΩ en serie de la placa analógica forman un filtro de τ = 1 ms, despreciable frente a la descarga del detector (0,47 s).
- **Desacople:** C8 / C15 (100 nF) en Vdd; C9 + C10 / C16 + C17 (1 µF + 100 nF) junto al pin Vref.
- **SPI ≤ 1 MHz** a 3,3 V (límite del MCP3204 con VDD = 2,7 V). Los MCP41010 aceptan hasta 10 MHz: el firmware configura la frecuencia por dispositivo.
- **MISO compartido.** Dout del MCP3204 queda en alta impedancia mientras CS está alto, así que U2 y U3 comparten MISO. Los MCP41010 no tienen salida SO. El ERC marca "Output–Output" entre los dos Dout; en el proyecto ese caso está bajado a advertencia porque es intencional.

## 5. SPI hacia las placas: 33 Ω en serie (R1–R8)

SCK y MOSI de cada línea pasan por 33 Ω (R1/R2 = L1, R3/R4 = L2, R5/R6 = L3, R7/R8 = N). El cable plano a cada placa es una línea de transmisión corta sin terminar. Sin resistencia, los flancos de SCK rebotan y pueden contar como un pulso de reloj extra en el MCP41010. Los 33 Ω, junto con la impedancia de salida del RP2350B, se acercan a la impedancia del cable y amortiguan el rebote. Además, cada línea queda aislada de las otras: un cortocircuito en un cable no tira abajo el SPI de todas.

## 6. I2C

Pull-ups de 4,7 kΩ en SDA y SCL (R10, R11). **No colocarlos si la placa del RP2350B ya tiene pull-ups** en ese bus. Con 4,7 kΩ y unos 100 pF de bus, 400 kHz funciona bien.

## 7. Consumo estimado (4 placas analógicas)

| Riel | Consumidor | Corriente |
|---|---|---|
| 3,3 V | Divisor de VBIAS, 4 × 165 µA | 0,7 mA |
| 3,3 V | MCP41010, 8 × ~1 µA | < 0,1 mA |
| 3,3 V | MCP3204, 2 × ~0,4 mA (convirtiendo) | 0,8 mA |
| 3,3 V | MCP23017 activo + un CS en bajo (0,33 mA) + I2C (0,7 mA en bajo) | ~2 mA |
| **3,3 V** | **Total** | **~4 mA** |
| 5 V | 3 LM358 por línea (~0,7 mA c/u) | 4 × 2 mA |
| 5 V | Pull-downs de 1 kΩ en las salidas de los LM358 (~1,65 mA c/u) | 4 × ~8 mA |
| **5 V** | **Total** | **~45 mA** |

C1 y C2 (10 µF) son la reserva de 3,3 V y 5 V en la entrada.

## 8. Notas para el firmware (Zephyr)

- Zephyr tiene drivers para los dos integrados: `microchip,mcp23017` (familia `mcp23xxx`, como controlador GPIO) y `microchip,mcp3204` (familia `mcp320x`, subsistema ADC).
- Los `cs-gpios` del bus SPI pueden apuntar a pines del MCP23017 (`<&mcp23017 0 GPIO_ACTIVE_LOW>`, etc.). Así los MCP3204 y los MCP41010 se declaran como dispositivos SPI normales.
- Cada transacción SPI cuesta dos escrituras I2C extra (bajar y subir el CS): unos 100–200 µs a 400 kHz. Alcanza para cambiar ganancias y para leer los picos cada ~100 ms. No sirve para muestreo rápido, que igual lo hace el ADC del RP2350B.
- Hay que configurar el MCP23017 (pines como salida en alto) antes de la primera transacción SPI. Hasta ese momento los pull-ups mantienen todo deseleccionado.

## 9. Lista de componentes

J1–J6 usan footprints de [tp_digital.pretty](tp_digital.pretty): los estándar con pads de 2,0 mm y agujero de 1,0 mm, para agujerear a mano.

R1, R3, R5, R6, R7, R10 y R20, y los capacitores C4, C7, C12 y C13, tienen pistas que pasan entre sus pads. Por eso las resistencias son 1206: no reemplazarlas ni a ellas ni a los capacitores por un tamaño menor (ver [placa_pcb.md](placa_pcb.md), sección 4).

| Ref. | Valor | Tipo | Función |
|---|---|---|---|
| U1 | MCP23017-E/SO | SOIC-28W | Expansor de E/S I2C (chip selects) |
| U2, U3 | MCP3204-CI/SL | SOIC-14 | ADC de picos (U2: L1 y L2, U3: L3 y N) |
| J1–J4 | 2x07 | IDC vertical, 2,54 mm | Placas analógicas L1, L2, L3, N |
| J5 | 2x13 | IDC vertical, 2,54 mm | RP2350B |
| J6 | 1x08 | Tira 2,54 mm | GPB2–GPB7 libres |
| R1–R8 | 33 Ω | 1206, 1 % | Serie en SCK y MOSI de cada línea |
| R9 | 10 kΩ | 1206, 1 % | Pull-up de RESET del MCP23017 |
| R10, R11 | 4,7 kΩ | 1206, 1 % | Pull-ups I2C (opcionales) |
| R12–R21 | 10 kΩ | 1206, 1 % | Pull-ups de los 10 chip selects |
| C1, C2 | 10 µF | 0805, X7R | Reserva de 3,3 V y 5 V |
| C3 | 100 nF | 0805, X7R | Desacople de U1 |
| C4–C7, C11–C14 | 100 nF | 0805, X7R | Entradas de los MCP3204 |
| C8, C15 | 100 nF | 0805, X7R | Desacople de Vdd de U2 y U3 |
| C9, C16 | 1 µF | 0805, X7R | Vref de U2 y U3 |
| C10, C17 | 100 nF | 0805, X7R | Vref de U2 y U3 |
