# Simulaciones

Modelos SPICE del front end analógico de [hardware/analog](../hardware/analog). Sirven para probar cambios en el circuito antes de tocar el esquemático.

Los circuitos son **esquemáticos de LTspice** (probados con LTspice 17.2.4 para macOS). Abrí un archivo `.asc` en LTspice y apretá Run. Los símbolos propios (`*.asy`) tienen que quedar en la misma carpeta que los esquemáticos, porque LTspice busca los símbolos primero al lado del `.asc`.

| Esquemático | Análisis | Qué graficar |
|---|---|---|
| `current_channel/current_ac.asc` | Respuesta en frecuencia del canal de corriente para cada ganancia | `V(norm)`: tensión en el ADC dividida por la ganancia ideal |
| `current_channel/current_tran.asc` | Corriente de carga distorsionada, cuatro ganancias, con y sin diodos de protección | `V(adc)` contra `V(ideal)` |
| `current_channel/current_crossover.asc` | Distorsión de cruce del LM358 con y sin pull-downs | `V(adc)`, después View → FFT |
| `voltage_channel/voltage_ac.asc` | Respuesta en frecuencia del canal de tensión para cada ganancia | `V(norm)` |
| `voltage_channel/voltage_tran.asc` | Red con armónicas de 220 a 650 V rms, con y sin diodos de protección | `V(adc)` contra `V(ideal)` |
| `front_end/front_end_tran.asc` | Los dos canales con **un solo buffer de VBIAS**: diafonía y fase V–I | `V(adc_i)`, `V(adc_v)`, `V(vbias)` |
| `current_channel/current_peak.asc` | Detector de pico de corriente en régimen permanente, de 2 a 140 A rms | `V(ipk)-V(vbias)` contra `V(pkideal)` |
| `current_channel/current_peak_event.asc` | Arranque de 5 a 30 A rms durante 10 ms a 500 ms, con G = 8: el canal recorta y el detector da el nivel | `V(ipk)-V(vbias)`, `V(pkideal)`, `V(adc)` |
| `voltage_channel/voltage_peak.asc` | Detector de pico de tensión en régimen permanente, de 110 a 800 V rms | `V(vpk)-V(vbias)` contra `V(pkideal)` |
| `voltage_channel/voltage_peak_event.asc` | Sobretensión de 220 a 330 V rms durante 10 ms a 500 ms | `V(vpk)-V(vbias)`, `V(pkideal)` |

Todos los esquemáticos incluyen el detector de pico (abajo del canal, conectado por las etiquetas `ct`/`vt` y `vbias`), igual que en KiCad.

Los resultados de las mediciones y de Fourier aparecen en View → SPICE Error Log (los pasos de `.step` se listan en orden ascendente). Todas las redes tienen nombre (`ct`/`vt`, `inp`, `inn`, `gout`, `x`, `fn`, `fout`, `adc`, `vbias`, `bbuf`; en `front_end` con sufijo `_i` o `_v`). La salida del detector de pico es `ipk` o `vpk`, y en las simulaciones del detector `pkideal` es el pico ideal en el secundario. Los archivos de salida de LTspice están en el `.gitignore`.

Para cambiar la ganancia o un valor, editá la línea `.param` o el componente. Cada esquemático tiene su copia del circuito: si cambiás uno, copiá el cambio en los demás.

### Parámetros comunes

| Parámetro | Qué hace |
|---|---|
| `D` (`DI` y `DV` en `front_end`) | Código del MCP41010 del canal: ganancia = 256/D. En `voltage_tran`, D = 32 (G = 8) hasta 300 V y D = 64 (G = 4) arriba |
| `V1` | Tensión de red en V rms (canal de tensión) |
| `RPD` | Pull-downs de salida de los LM358 (1k colocados, 1G quitados) |
| `VOS` | Offset de entrada del LM358 de la etapa de ganancia |
| `RD` | Resistencia en serie con cada diodo del BAT54S (`DL` a GND, `DH` a 3,3 V): 1m = colocados, 1G = quitados |
| `I1`, `K`, `T0`, `TE` | Simulaciones de eventos: nivel normal, factor del evento, inicio y duración |
| `RBI`, `RCBI` | Salida de VBIAS. `RBI=1m RCBI=1G`: el seguidor maneja VBIAS directo (diseño actual). `RBI=47 RCBI=1m`: aislación de 47 Ω + 1 µF (diseño anterior) |

## Estructura

| Ruta | Contenido |
|---|---|
| `lib/lm358_behavioral.lib` | LM358 comportamental escrito para este proyecto: ganancia, ancho de banda, slew rate, offset, corriente de polarización, excursión de salida y distorsión de cruce |
| `lib/sct013_000.lib` | Transformador de corriente YHDC SCT-013-000 (100 A : 50 mA), lineal |
| `lib/zmpt101b.lib` | Transformador de tensión ZMPT101B (2 mA : 2 mA), lineal |
| `lib/mcp41010.lib` | Potenciómetro digital MCP41010; el código del cursor es el parámetro `D` |
| `lib/bat54.lib` | Schottky BAT54 (cada mitad del BAT54S), ajuste a la curva típica de la hoja de datos (no es el modelo del fabricante) |
| `current_channel/`, `voltage_channel/`, `front_end/` | Esquemáticos y símbolos (`SCT013_000.asy`, `ZMPT101B.asy`, `MCP41010.asy`) |

## Circuito simulado

```
Corriente: SCT-013-000 ── burden 15 Ω ── 1 k / 1 nF ── LM358 no inversor, G = 256/D (MCP41010) ──┐
Tensión:   red ── 6 × 47 kΩ ── ZMPT101B ── burden 100 Ω ── 1 k / 1 nF ── LM358 no inversor, G = 256/D (MCP41010) ──┤
                                                    ┌──────────────────────────────────────────────────────┘
                                                    └── LM358 pasabajos MFB (f0 2,6 kHz, Q 0,75, ganancia −1) ── 1 k ──┬── ADC
                                                                                                                      └── BAT54S a GND y 3,3 V
VBIAS = 3,3 V / 2 → 10 µF → seguidor LM358 → VBIAS (compartido por los dos canales)

Detector de pico (uno por canal, desde el secundario ct / vt, referido a VBIAS):
ct / vt ── BAT54 ── 2,2 k ──┬── 1 k ──┬── 10 k ──┬── ipk / vpk
              470 n ∥ 1 MΩ  100 n     └── BAT54S a GND y 3,3 V
                (a VBIAS)    (a VBIAS)
```

Es el mismo circuito que el esquemático de KiCad: hojas `Canal IAC` y `Canal VAC`, y el buffer de VBIAS (U1A) en la hoja principal.

**Canal de tensión (ganancia variable, igual que el de corriente):** sensibilidad 100 Ω / 282 kΩ = 0,355 mV por volt de red antes de la etapa de ganancia; fondo de escala = 2790 V rms / G.

| Tensión de red | Código (G) | Corriente primaria | Potencia total en las 6 × 47 kΩ | Tensión por resistencia | Pico en el ADC (desde VBIAS) |
|---|---|---|---|---|---|
| 220 V | D = 32 (G = 8) | 0,78 mA | 0,17 W | 37 V | 0,88 V |
| 380 V | D = 64 (G = 4) | 1,35 mA | 0,51 W | 63 V | 0,76 V |
| 607 V | D = 64 (G = 4) | 2,15 mA | 1,31 W | 101 V | 1,22 V |
| 1000 V (pico breve) | D = 128 (G = 2) | 3,55 mA | — | 167 V | 1,00 V |

- Los dos canales usan la misma etapa (LM358 + MCP41010 como divisor), así que con el mismo código su desfasaje es idéntico.
- El ZMPT101B queda en 1,35 mA a 380 V (nominal 2 mA, lineal hasta 10 mA).
- Sobretensiones sostenidas por encima de ~650 V superan los 0,25 W de cada resistencia 1206; los picos de arranque son breves.

## Resultados (LTspice 17.2.4, 2026-10-20)

Valores normalizados (resistencias E24 1 %, capacitores E12), filtro MFB plano hasta la armónica 31 y protección del ADC con BAT54S. Los cálculos y la justificación de cada componente están en [hardware/analog/calculos_front_end.md](../hardware/analog/calculos_front_end.md).

### 1. Protección del ADC: BAT54S en lugar del Zener

El Zener BZV55C3V3 tenía un codo blando: conducía ~10 µA a 1,65 V y ~1 mA a 2,9 V, y a través de la resistencia de 1 kΩ recortaba la parte alta de la señal (hasta −12 % de fundamental y 9,5 % de 2da armónica). Se reemplazó por un **BAT54S** (dos Schottky en serie, a GND y a 3,3 V). Dentro del rango de la señal (0,25 a 3,05 V) los dos diodos quedan en inversa y no conducen:

| Caso | Fundamental vs. ideal, con BAT54S | Fundamental vs. ideal, sin diodos | 2da armónica, con BAT54S |
|---|---|---|---|
| Corriente, G = 16 (pico ±1,36 V) | −0,37 % | −0,37 % | 0,05 % |
| Tensión, 220 V (G = 8) | −0,08 % | −0,08 % | 0,02 % |
| Tensión, 607 V (G = 4) | −0,08 % | −0,08 % | 0,02 % |

Los resultados con y sin los diodos coinciden en todas las corridas (`RD` = 1m o 1G). La red de 380 V con 3,6 % de THD real se mide con 3,59 % (ideal 3,58 %). El error de −0,37 % del canal de corriente es el del TC (magnetización) y el de VBIAS, no de la protección.

Las corridas incluyen el detector de pico. La diferencia de 0,02 % respecto de las corridas sin detector es la carga del detector mientras su capacitor se carga en la ventana de 100 ms.

### 2. Respuesta en frecuencia

`V(norm)` es la tensión en el ADC dividida por la ganancia ideal (0 dB y 0° sería un canal perfecto).

| Canal, código | 50 Hz | 1550 Hz (armónica 31) | −3 dB | 5,12 kHz (Nyquist) | 8,69 kHz (alias de 1550 Hz) |
|---|---|---|---|---|---|
| Corriente, D = 255 (G = 1) | −0,02 dB, +2,32° | −0,15 dB, −51,6° | 2,76 kHz | −11,8 dB | −21,0 dB |
| Corriente, D = 64 (G = 4) | −0,02 dB, +2,31° | −0,15 dB, −52,0° | 2,76 kHz | −11,8 dB | −21,0 dB |
| Corriente, D = 16 (G = 16) | −0,02 dB, +2,26° | −0,16 dB, −53,6° | 2,76 kHz | −11,9 dB | −21,1 dB |
| Corriente, D = 8 (G = 32) | −0,02 dB, +2,19° | −0,18 dB, −55,7° | 2,74 kHz | −12,1 dB | −21,7 dB |
| Tensión, D = 255 (G = 1) | −0,006 dB, −1,16° | −0,16 dB, −51,8° | 2,76 kHz | −11,8 dB | −21,0 dB |
| Tensión, D = 64 (G = 4) | −0,006 dB, −1,17° | −0,16 dB, −52,2° | 2,76 kHz | −11,8 dB | −21,0 dB |
| Tensión, D = 32 (G = 8) | −0,006 dB, −1,19° | −0,16 dB, −52,7° | 2,76 kHz | −11,9 dB | −21,0 dB |
| Tensión, D = 8 (G = 32) | −0,008 dB, −1,29° | −0,19 dB, −55,9° | 2,74 kHz | −12,1 dB | −21,7 dB |

- **Banda plana:** hasta la armónica 31 (1550 Hz) la caída es de −0,15 a −0,19 dB (1,7 a 2,1 %) y el firmware la corrige.
- **Diferencia de fase V–I a 50 Hz: 3,4°** (el TC adelanta 3,8°, el ZMPT101B 0,3°). Se corrige con la calibración de fase; sin corregir, con FP = 0,5 daría un error de potencia activa de ~10 %.
- Los dos canales tienen el mismo filtro y la misma etapa de ganancia: con el mismo código su desfasaje se cancela. Lo que varía con el código es el ancho de banda del LM358: 4,1° en 1550 Hz entre G = 1 y G = 32. Si V e I usan códigos distintos, se corrige esa diferencia.
- **Antialiasing** (fs = 10 240 Hz por canal): lo que se pliega sobre la banda de 0 a 1550 Hz está entre 8,69 y 11,79 kHz y llega atenuado ≥ 21 dB.

### 3. Un solo buffer de VBIAS para los dos canales (`front_end_tran.asc`)

380 V en el canal de tensión (D = 64, G = 4) y el canal de corriente con G = 32, sin diodos de protección.

| Salida de VBIAS | Diafonía V → I (corriente = 0) | Error extra en I con 1 A en fase con V | Error extra en V |
|---|---|---|---|
| 47 Ω + 1 µF | 3,6 mV (1,1 %) | −1,5 % | −0,7 % |
| Seguidor directo, sin capacitor (diseño actual) | < 0,05 mV | 0 (referencia) | 0 (referencia) |

Con 47 Ω, la corriente alterna de las redes de realimentación pasa por esa resistencia: la tensión de un canal aparece en el otro **en fase**, y eso es un error de potencia que no se corrige con una calibración de ganancia. Con el seguidor manejando VBIAS directo, la impedancia de salida a 50 Hz es de miliohms y la diafonía desaparece. Con la ganancia variable en V, la red de realimentación pasó de 46 kΩ a 10 kΩ y la diafonía con 47 Ω se cuadruplicó (antes 0,9 mV): otra razón para el seguidor directo.

En KiCad quedó así: R4 = 0 Ω y C26 marcado "sin colocar". **La estabilidad no se puede verificar con el modelo comportamental** (tiene un solo polo). VBIAS solo tiene cargas resistivas y unos pocos pF, así que el seguidor debería ser estable, pero hay que mirar VBIAS con el osciloscopio en la primera placa. Si oscila: R4 = 47 Ω y colocar C26, aceptando la diafonía de la tabla.

### 4. Distorsión de cruce (`current_crossover.asc`)

Senoidal pura de 2 A rms con ganancia 32.

| Pull-downs de salida | THD en V(adc) |
|---|---|
| 1 kΩ colocados | 0,08 % |
| Quitados | 0,43 % |

### 5. Detector de pico (`current_peak*.asc`, `voltage_peak*.asc`)

Detector pasivo de media onda tomado del secundario: BAT54, 2,2 kΩ, 470 nF ∥ 1 MΩ a VBIAS, filtro 1 kΩ / 100 nF y 10 kΩ + BAT54S hacia el ADC. Cálculo en [calculos_front_end.md](../hardware/analog/calculos_front_end.md), sección 8.

**Régimen permanente.** Pico real en el secundario contra la lectura del detector (salida media menos VBIAS), después de 1,3 s:

| Corriente | Pico | Lectura | Lectura / pico | | Tensión | Pico | Lectura | Lectura / pico |
|---|---|---|---|---|---|---|---|---|
| 2 A | 21,2 mV | 2,3 mV | 11 % | | 110 V | 55,1 mV | 14,3 mV | 26 % |
| 5 A | 52,9 mV | 13,3 mV | 25 % | | 220 V | 110 mV | 46,1 mV | 42 % |
| 10 A | 106 mV | 43,3 mV | 41 % | | 380 V | 190 mV | 104 mV | 54 % |
| 30 A | 317 mV | 206 mV | 65 % | | 607 V | 304 mV | 194 mV | 64 % |
| 100 A | 1058 mV | 869 mV | 82 % | | 800 V | 401 mV | 276 mV | 69 % |
| 140 A | 1481 mV | 1263 mV | 85 % | | | | | |

La curva no es lineal porque la caída del BAT54 pesa más con poca señal: el firmware la convierte con una tabla de calibración. El ripple es del 3 al 4 % de la lectura.

**Eventos.** Máximo del detector durante el evento, comparado con su lectura en régimen permanente al nivel del evento:

| TE | Arranque 5 → 30 A, G = 8 | Sobretensión 220 → 330 V |
|---|---|---|
| 10 ms (medio ciclo positivo) | 61 % | 69 % |
| 20 ms | 61 % | 69 % |
| 100 ms | 93 % | 90 % |
| 500 ms | 102 % | 102 % |

- 10 y 20 ms dan lo mismo: el detector de media onda solo ve el semiciclo positivo.
- Un segundo después del evento vuelve a la lectura previa.
- En el arranque, el canal principal (G = 8, fondo de escala 16,5 A rms) recorta durante el evento y el detector sigue dando el nivel.

**Nota para correr en batch:** LTspice 17.2.4 para macOS, en modo `-b`, mide `.meas ... FROM/TO` con el tiempo desplazado cuando `.tran` guarda desde un instante mayor que cero. Por eso estas simulaciones guardan desde t = 0.

## Limitaciones de los modelos

- El LM358 es un modelo comportamental, no el del fabricante (la descarga de TI no está disponible para scripts). TI y onsemi publican modelos PSpice con el mismo orden de pines (`IN+ IN- VCC VEE OUT`): agregá su `.lib` con un `.include` y cambiá el valor de los operacionales de `LM358B` al nombre de su subcircuito. Como tiene un solo polo, **no predice la estabilidad** con cargas capacitivas.
- El SCT-013 y el ZMPT101B son lineales: no incluyen saturación ni error de relación. Sus resistencias de bobinado, inductancias y capacidades son estimaciones ajustadas a los datos publicados (fase del SCT-013 medida por OpenEnergyMonitor; fase ≤ 20′ del ZMPT101B con 100 Ω). Medí tus piezas y actualizá los parámetros.
- El BAT54 es un ajuste a la curva típica (VF y capacidad). La fuga inversa real crece mucho con la temperatura (hasta unos µA a 85 °C), lo que sobre 1 kΩ da unos pocos mV de offset. En el detector de pico, la fuga circula por la descarga de 1 MΩ y cambia bastante la lectura: la curva del detector se mide en banco y no se toma de la simulación.
- El MCP41010 es estático: no modela la interfaz SPI, los glitches al cambiar la ganancia ni el INL/DNL.
- Los modelos usan elementos SPICE estándar, salvo `Type=schottky` en el modelo del diodo e `if()` en `front_end`, que son propios de LTspice.
