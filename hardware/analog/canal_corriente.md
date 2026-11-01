# Canal de medición de corriente: diseño y justificación

Este documento explica el circuito del canal de corriente del analizador: qué hace cada bloque, por qué tiene esos valores y qué queda por definir. Está pasado al esquemático de KiCad ([current_channel.kicad_sch](current_channel.kicad_sch)). Los cálculos de todos los componentes del front end, con los valores normalizados (E24/E12), están en [calculos_front_end.md](calculos_front_end.md).

El circuito está simulado en LTspice: ver [simulations/current_channel](../../simulations/current_channel) y los resultados en [simulations/README.md](../../simulations/README.md).

## 1. Requisitos y supuestos

| Parámetro | Valor | Origen |
|---|---|---|
| Sensor | YHDC SCT-013-000, 100 A : 50 mA, salida en corriente | Elegido |
| Red | 220 V, 50 Hz, monofásica | Requisito |
| Rango de corriente | ~0,5 A a 100 A rms | Requisito del analizador |
| Ancho de banda de interés | Hasta la armónica 31 (1550 Hz), plano (≤ 0,5 dB) | Requisito |
| ADC | Interno del RP2350B, 12 bits, 0 a 3,3 V, entrada simple | docs/architecture.md |
| Frecuencia de muestreo | 10 240 Hz por canal | Firmware (`CONFIG_ADC_STREAM_PER_CHANNEL_SAMPLE_RATE_HZ`) |
| Alimentaciones | 5 V (operacionales) y 3,3 V (lógica, MCP41010, ADC) | Placa actual |
| Operacional | LM358 | Elegido por disponibilidad para el primer prototipo |
| Ganancia variable | MCP41010 (10 kΩ, 256 posiciones, SPI). Su CS (CS_I) lo maneja el MCP23017 de la [placa digital](../digital/README.md) | Placa actual |

## 2. Diagrama del canal

```
 SCT-013-000
     tip ────┬──── ct ── RIN 1k ──┬──── (+) U1 ── gout ── U2: pasabajos MFB ── fout ── 1k ──┬── ADC
             │                    │     no inversor        2do orden, 2,6 kHz,
          RB 15 Ω              CIN 1 nF  G = 256/D          ganancia −1      └── BAT54S
             │                    │     (MCP41010 en la
  sleeve ────┴──── VBIAS        VBIAS    realimentación)

 MCP41010:  A = gout,  W = entrada (−) de U1,  B = VBIAS
 VBIAS:     3,3 V ── 10k ──┬── 10k ── GND  →  U3 seguidor ── 47 Ω ──┬── VBIAS
                           └── 1 µF ── GND                          └── 1 µF ── GND
 Cada salida de LM358 (U1, U2, U3) tiene un pull-down de 1 kΩ a GND.

 Detector de pico (pasivo, del secundario, referido a VBIAS):
     ct ── BAT54 ── 2,2k ──┬── 1k ──┬── 10k ──┬── IPK (MCP3204 de la placa digital)
                  470 nF ∥ 1M    100 nF       └── BAT54S
                           │        │
                         VBIAS    VBIAS
```

Los tres operacionales (U1, U2, U3) son LM358 alimentados a 5 V. Hacen falta dos encapsulados para este canal; con el canal de tensión, tres en total (seis secciones: buffer de VBIAS, dos etapas de ganancia, dos filtros y una libre).

## 3. Idea general: todo referido a VBIAS

Con una sola fuente, la señal alterna no puede estar centrada en 0 V. Todo el canal trabaja alrededor de **VBIAS = 1,65 V**, la mitad del rango del ADC, que funciona como "tierra de señal". Cada nodo se puede escribir como VBIAS más una componente de señal:

```
V(ct)   = VBIAS + v_ct                     v_ct = i_secundario · RB
V(gout) = VBIAS + G · v_ct                 G = 256/D
V(fout) = VBIAS − H(f) · G · v_ct          H(f) = pasabajos, H(0) = 1
```

El ADC lee 1,65 V más la señal invertida y filtrada. El firmware resta el valor medio (o lee VBIAS en otro canal del ADC) y cambia el signo.

## 4. Sensor: SCT-013-000

### 4.1 Datos

| Parámetro | Valor | Fuente |
|---|---|---|
| Relación | 100 A : 50 mA → N = 2000 | Hoja de datos |
| No linealidad | ±3 % entre 10 % y 120 % de la corriente nominal | Hoja de datos |
| Error de relación medido | Entre −2 % y +2 %, típicamente +1,5 %, de 0,5 A a 120 A | Informe de OpenEnergyMonitor |
| Saturación | Empieza alrededor de 140 A | Informe de OpenEnergyMonitor |
| Error de fase medido | Algo más de 4° con burden de 22 Ω; casi 8° con 120 Ω | Informe de OpenEnergyMonitor |
| Rango de frecuencia | 50 Hz – 150 kHz | Hoja de datos (no verificado de forma independiente) |
| Protección | TVS interno que limita la tensión con el secundario abierto | Hoja de datos / OpenEnergyMonitor |

Referencias: [hoja de datos](https://cdn.sparkfun.com/assets/d/0/e/9/9/SCT-013-000-2.pdf), [informe de OpenEnergyMonitor](https://docs.openenergymonitor.org/electricity-monitoring/ct-sensors/yhdc-sct-013-000-ct-sensor-report.html).

### 4.2 Por qué la versión con salida en corriente

Las versiones SCT-013-0xx traen el burden adentro y dan 1 V a corriente nominal. La -000 da corriente y el burden va en la placa. Se eligió la -000 porque:

- El burden se elige: un valor bajo reduce el error de fase (ver 4.3).
- Se puede usar una resistencia de 0,1 % y bajo coeficiente térmico; la tolerancia del burden interno no se publica.
- Un mismo sensor cubre hasta 100 A.

### 4.3 Modelo y error de fase

El TC se modela, referido al secundario, como una fuente de corriente Ip/N en paralelo con la inductancia de magnetización Lm, seguida de la resistencia del bobinado Rw:

```
Ip/N ──┬── Rw ── tip
       Lm         RB (burden)
   ────┴───────── sleeve
```

Es un pasaaltos de primer orden. La salida adelanta a la corriente primaria en:

```
tan φ = (RB + Rw) / (ω · Lm)
```

Rw y Lm no están publicados. Con **Lm = 5,5 H y Rw = 100 Ω** el modelo reproduce las mediciones de OpenEnergyMonitor:

| Burden | φ calculado a 50 Hz | Medido |
|---|---|---|
| 22 Ω | 4,04° | "algo más de 4°" |
| 120 Ω | 7,26° | "casi 8°" |
| **15 Ω (diseño)** | **3,81°** | — |

Con 15 Ω: frecuencia de corte inferior f_L = (RB + Rw)/(2π·Lm) = **3,3 Hz**, y error de amplitud a 50 Hz de −0,019 dB (0,2 %).

**A revisar:** medir Rw con un óhmetro y la fase real con una carga resistiva. El error de fase cambia con la corriente (se midieron 2,5° a 4,5°), así que se calibra.

## 5. Burden: RB = 15 Ω

### 5.1 Sensibilidad

```
k = RB / N = 15 Ω / 2000 = 7,5 mV/A (rms)
```

### 5.2 Por qué 15 Ω

1. **Fondo de escala con ganancia 1.** La salida de los LM358 se limita a ±1,4 V alrededor de VBIAS (0,25 V a 3,05 V; ver sección 8.3). Con G = 1:

   ```
   I_max = 1,4 V / (√2 · 7,5 mV/A) = 132 A rms (senoidal)
   ```

   Cubre los 100 A nominales y llega al comienzo de la saturación del TC (~140 A).

2. **Error de fase bajo.** Con 15 Ω el adelanto es 3,8°, contra 4,0° con 22 Ω y 7,3° con 120 Ω.

3. **Disipación.**

   | Corriente primaria | I secundaria | Potencia en RB |
   |---|---|---|
   | 100 A | 50 mA | 37,5 mW |
   | 140 A (saturación) | 70 mA | 73,5 mW |

   Alcanza con una resistencia 1206 de 0,25 W. Se recomienda **0,1 %, ≤ 25 ppm/°C**, porque su tolerancia entra directo en la ganancia.

### 5.3 Ubicación

RB va **pegada al conector del TC (jack de 3,5 mm)**. Así el secundario nunca queda abierto, aunque falle el resto del circuito.

## 6. Conexión del TC a VBIAS

El secundario del TC es flotante. El sleeve se conecta a VBIAS para fijar el nivel de continua:

- La corriente del secundario circula solo por el lazo TC → RB → TC. **No entra corriente del TC a VBIAS.**
- La tensión en el tip queda montada sobre 1,65 V. Con 10 A: 5 mA × 15 Ω = 75 mV rms → V(ct) = 1,65 V ± 106 mV pico.
- Si el sleeve fuera a GND, los semiciclos negativos quedarían por debajo de 0 V, fuera del rango del LM358 y del ADC.

## 7. Filtro de RF de entrada: RIN = 1 kΩ, CIN = 1 nF

```
f_c = 1 / (2π · 1 kΩ · 1 nF) = 159 kHz
```

- **No es el filtro antialiasing.** Saca ruido de RF que entra por el cable del TC (1 m). A 1550 Hz agrega solo 0,6° de atraso.
- **RIN limita la corriente** hacia los diodos de protección de la entrada del LM358 ante transitorios (arranques de motores, descargas).
- **CIN va a VBIAS, no a GND.** El filtro actúa sobre la señal v_ct, que es la diferencia entre ct y VBIAS. El ruido presente en VBIAS aparece igual en los dos extremos de CIN y no se convierte en señal.
- **Error por corriente de polarización:** 45 nA típ. (250 nA máx.) × (1 kΩ + 15 Ω) = 46 µV (0,25 mV máx.). Despreciable frente al offset del LM358.
- CIN: **C0G/NP0**, por estabilidad térmica y porque no cambia con la tensión.

## 8. Etapa de ganancia: U1 + MCP41010

### 8.1 Topología

U1 es un **amplificador no inversor**. La señal entra por la entrada +. El MCP41010 se usa como **divisor de tensión** en la realimentación:

```
gout ── A ┐
          ├ R_AW = R_AB·(256−D)/256
          W ──── entrada (−) de U1
          ├ R_WB = R_AB·D/256
VBIAS ── B ┘
```

### 8.2 Ganancia

La tensión en el cursor, que no toma corriente, es:

```
V(W) = VBIAS + (V(gout) − VBIAS) · D/256
```

El operacional iguala V(−) = V(+) = VBIAS + v_ct, entonces:

```
V(gout) = VBIAS + (256/D) · v_ct          G = 256/D
```

Propiedades de esta conexión:

| Propiedad | Por qué |
|---|---|
| **Solo se amplifica la señal** | El extremo B está en VBIAS, así que los 1,65 V salen con ganancia 1. Si B fuera a GND: V(gout) = G·(1,65 V + v_ct), que satura con cualquier G > 2. |
| **La ganancia depende solo de la relación D/256** | La tolerancia de ±20 % de R_AB se cancela. La deriva térmica de la relación es de pocos ppm/°C, contra ~800 ppm/°C en modo reóstato. |
| **La resistencia del cursor (R_W ≈ 52 Ω) no influye** | No circula corriente por el cursor (va a una entrada del operacional). |
| **Corriente por el potenciómetro** | Solo alterna: (V(gout) − VBIAS)/10 kΩ ≤ 1,4 V/10 kΩ = 0,14 mA. Muy por debajo del límite de 1 mA. |

El esquema anterior usaba el MCP41010 como reóstato en un inversor (G = R_pot/R): la ganancia dependía del valor absoluto del potenciómetro y de la red de polarización, y la corriente de señal pasaba por el cursor. Ver [REVIEW.md](REVIEW.md), puntos 4 y 10.

### 8.3 Límites del LM358 que fijan el rango

| Límite (5 V, 25 °C) | Consecuencia | Decisión |
|---|---|---|
| Salida máxima ≈ Vcc − 1,5 V = 3,5 V | Puede superar los 3,3 V del ADC y del MCP41010 | Excursión útil de ±1,4 V alrededor de VBIAS (hasta 3,05 V) |
| Salida mínima limpia ≈ 0,23 V (con pull-down, ver sección 10) | Por debajo aparece distorsión de cruce | Mínimo de 0,25 V; el auto-rango mantiene la señal más arriba |
| Offset: 2 mV típ., 7 mV máx. | Se multiplica por G: 64 mV típ. y 224 mV máx. con G = 32 | **G máxima = 32** (D ≥ 8). El firmware resta la media. |
| GBW ≈ 0,7 MHz | Ancho de banda = GBW/G: 21,9 kHz con G = 32 | Aceptable; ver tabla de 8.4 |
| Slew rate 0,3 V/µs | f máx. a 1,4 V pico = 0,3 V/µs / (2π · 1,4 V) = 34 kHz | Sin problema |

### 8.4 Códigos de ganancia usados

| D | G | I máx. (rms, senoidal) | Señal con 0,5 A | Ancho de banda U1 | Fase de U1 a 50 Hz | Fase de U1 a 1550 Hz |
|---|---|---|---|---|---|---|
| 255 | 1,004 | 132 A* | 3,75 mV rms | 700 kHz | 0,00° | −0,1° |
| 128 | 2 | 66 A | 7,5 mV rms | 350 kHz | −0,01° | −0,3° |
| 64 | 4 | 33 A | 15 mV rms | 175 kHz | −0,02° | −0,5° |
| 32 | 8 | 16,5 A | 30 mV rms | 87,5 kHz | −0,03° | −1,0° |
| 16 | 16 | 8,2 A | 60 mV rms | 43,8 kHz | −0,07° | −2,0° |
| 8 | 32 | 4,1 A | 120 mV rms | 21,9 kHz | −0,13° | −4,1° |

\* El TC satura alrededor de 140 A y está especificado hasta 120 A.

- Cada escalón duplica la ganancia. Con el ADC de 12 bits (LSB = 0,81 mV), 0,5 A con G = 32 dan 120 mV rms ≈ 150 LSB rms.
- La fase de U1 a 1550 Hz varía 3,9° entre G = 1 y G = 32. **Hay que calibrar la fase para cada código** si interesan las armónicas.
- Para corrientes con armónicas, I máx. es menor: el auto-rango tiene que mirar el **pico**, no el valor eficaz.

### 8.5 Auto-rango (firmware)

- Subir de rango (bajar G) si |pico − VBIAS| > 1,3 V (93 % de la excursión).
- Bajar de rango (subir G) si |pico − VBIAS| < 0,6 V durante un ciclo completo. Al duplicar la ganancia la señal queda en 1,2 V < 1,3 V, así que no oscila entre rangos.
- Descartar un ciclo de red (20 ms) después de cada cambio: el cursor puede generar un glitch al conmutar.
- Si la salida recorta, el detector de pico (IPK, tomado del secundario antes de la etapa de ganancia) da el nivel real y permite ir directo al código correcto. El firmware lo lee con el MCP3204 de la placa digital. Lee desde ~5 A, justo donde G = 32 empieza a recortar (ver [calculos_front_end.md](calculos_front_end.md), sección 8).

### 8.6 Cuidados con el MCP41010

| Riesgo | Detalle | Medida |
|---|---|---|
| **D = 0** | R_WB = 0: realimentación nula, U1 a lazo abierto, salida a un riel | El driver nunca escribe D < 8 |
| **Comando de shutdown (0x21)** | Desconecta el terminal A: también abre el lazo | Usar solo el comando de escritura (0x11 + dato) |
| Encendido | El cursor arranca en 0x80 (G = 2), que es seguro | — |
| INL/DNL ±1 LSB máx. | Error de ganancia de hasta ±12,5 % con D = 8 (≈ ±3 % típ.) | Calibrar la ganancia de cada código |
| Tensión en los terminales | El MCP41010 va a 3,3 V y la salida de U1 puede llegar a 3,5 V en sobrecarga | **A verificar:** máximo absoluto de los terminales en la hoja de datos (recuerdo VDD + 1,0 V) |
| Capacidades del potenciómetro | ~5–10 pF con ≤ 2,5 kΩ de Thévenin en el cursor: polo > 6 MHz | Despreciable |

No se debe poner ningún capacitor en la realimentación de U1: haría que el ancho de banda y la fase dependan de la ganancia. Todo el filtrado está en la etapa siguiente.

## 9. Filtro antialiasing: U2, pasabajos MFB de 2do orden

### 9.1 Por qué una etapa separada con valores fijos

En el esquema anterior el filtro MFB compartía componentes con la ganancia (el potenciómetro era la resistencia de realimentación del filtro), así que la frecuencia de corte cambiaba con la ganancia (REVIEW.md, punto 8). Separado y con valores fijos, el filtro es **idéntico en el canal de tensión y en el de corriente**, y sus desfasajes se cancelan al calcular la potencia.

### 9.2 Topología

```
              ┌────────────── R2 ──────────────┐
              │                                │
gout ── R1 ── X ──── R3 ────┬──── (−)          │
              │             │          U2 ─────┴── fout
             C1             │   VBIAS ─(+)     │
              │             └────── C2 ────────┘
             GND
```

Transferencia:

```
H(s) = −(1/(R1·R3·C1·C2)) / [ s² + s·(1/C1)·(1/R1 + 1/R2 + 1/R3) + 1/(R2·R3·C1·C2) ]

Ganancia en continua = −R2/R1
ω0 = 1/√(R2·R3·C1·C2)
Q  = ω0 · C1 / (1/R1 + 1/R2 + 1/R3)
```

La entrada + está en VBIAS, así que el filtro invierte alrededor de VBIAS. En continua, V(fout) = VBIAS − (V(gout) − VBIAS) = VBIAS (V(gout) en reposo es VBIAS). **No agrega offset**, solo lo conserva.

### 9.3 Valores

Objetivo: **plano hasta la armónica 31** (≤ 0,5 dB en 1550 Hz en el peor caso de tolerancias), ganancia −1, valores E24/E12. El cálculo completo, la búsqueda de valores y el análisis de tolerancias están en [calculos_front_end.md](calculos_front_end.md), sección 7.

**R1 = R2 = 10 kΩ, R3 = 3,6 kΩ, C1 = 22 nF, C2 = 4,7 nF → f0 = 2609 Hz, Q = 0,755.**

| Frecuencia | Ganancia | Fase (sin contar la inversión) |
|---|---|---|
| 50 Hz | 0,00 dB | −1,5° |
| 1550 Hz (armónica 31) | −0,16 dB | −50,6° |
| 2770 Hz | −3,0 dB | — |
| 5120 Hz (Nyquist) | −11,7 dB | −137,6° |
| 8690 Hz (alias de 1550 Hz) | −20,8 dB | −156,4° |

- **Fase en la banda:** −51° en 1550 Hz, igual en los dos canales: se cancela en la potencia. La caída de −0,16 dB se corrige en el firmware.
- **Antialiasing:** con fs = 10 240 Hz, lo que se pliega sobre 0–1550 Hz (8,69 a 11,79 kHz) llega atenuado ≥ 21 dB.

### 9.4 Sensibilidad a tolerancias (diferencia de fase entre canales)

| Tolerancia de los capacitores | Diferencia de fase a 50 Hz | A 1550 Hz (Monte Carlo, p99) |
|---|---|---|
| ±5 % | 0,14° | 4,5° |
| ±2 % | 0,06° | 2,0° |

- Capacitores **C0G/NP0**: ±30 ppm/°C y sin efecto de la tensión.
- Resistencias **1 %**. La ganancia del filtro, −R2/R1, puede variar hasta ±2 %; se calibra junto con el resto.

## 10. Pull-downs de 1 kΩ en las salidas (RPD)

La etapa de salida del LM358 es clase B con una fuente interna de 50 µA para absorber corriente. Cuando la carga pide absorber más que eso, la salida cruza una zona muerta y aparece **distorsión de cruce**, que se vería como armónicas falsas.

Con un pull-down a GND, la salida siempre entrega corriente y no cruza la zona muerta. La carga de U1 hacia VBIAS es el potenciómetro (10 kΩ) en paralelo con la entrada del filtro (R1 = 10 kΩ) = 5 kΩ. La salida no cruza mientras:

```
V/1 kΩ + 50 µA ≥ (1,65 V − V) / 5 kΩ   →   V ≥ 0,23 V
```

Por eso la excursión mínima es 0,25 V (sección 8.3).

- Corriente máxima entregada (a 3,05 V): 3,05 mA + 0,35 mA ≈ 3,4 mA. El LM358 entrega más de 20 mA.
- Costo: unos 1,65 mA promedio por salida desde los 5 V.
- Simulado con G = 32 y una senoidal pura: **THD de 0,08 % con los pull-downs y 0,43 % sin ellos.**

## 11. VBIAS: divisor + seguidor (U3)

### 11.1 Divisor

- 10 kΩ / 10 kΩ desde 3,3 V → 1,65 V. Con 1 %: ±13 mV, sin importancia porque el firmware resta la media.
- CBD = 1 µF con 5 kΩ: f_c = 31,8 Hz. **Recomendación: subirlo a 10 µF** (f_c = 3,2 Hz). El ruido de VBIAS llega al ADC con ganancia 1 (V(fout) = VBIAS − señal), así que conviene filtrar bien el ruido de los 3,3 V. Costo: la VBIAS tarda unos 250 ms en estabilizarse (5τ) al encender.

### 11.2 Por qué hace falta el seguidor

VBIAS recibe corriente alterna: la del potenciómetro (hasta ±0,14 mA). Sin buffer, sobre los 5 kΩ del divisor eso movería VBIAS hasta **±0,7 V** al ritmo de la señal (±0,3 V a 50 Hz, donde el capacitor de 1 µF ya baja la impedancia a ~1,9 kΩ). Como VBIAS es la referencia de todo el canal, la medición quedaría arruinada.

### 11.3 RBI = 47 Ω y CBI = 1 µF

- La resistencia en serie aísla al LM358 del capacitor de 1 µF, que podría hacerlo oscilar. El capacitor da baja impedancia en alta frecuencia (f_c = 3,4 kHz).
- **Efecto secundario detectado en la simulación:** a 50 Hz la impedancia de salida de VBIAS es ≈ 47 Ω. La corriente del potenciómetro deja en VBIAS una copia de la señal de 47 Ω/10 kΩ = **0,47 %**, que aparece como error de ganancia. Es plano en frecuencia y se calibra.
- Alternativas para revisar: tomar la realimentación del seguidor después de la resistencia de 47 Ω (con compensación), o medir VBIAS con otro canal del ADC. **La estabilidad no se puede verificar con el modelo comportamental del LM358**; hace falta el modelo del fabricante o una prueba en banco.

## 12. Salida al ADC: 1 kΩ en serie y BAT54S

- La resistencia de 1 kΩ limita la corriente hacia el ADC y hacia los diodos si la salida del LM358 supera los 3,3 V (puede llegar a 3,5 V en sobrecarga).
- **BAT54S** (dos Schottky en serie, SOT-23) a GND y a 3,3 V: limita la entrada del ADC a −0,3 V … 3,6 V. Dentro del rango de la señal (0,25 a 3,05 V) los dos diodos están en inversa y no conducen. Reemplaza al Zener BZV55C3V3, cuyo codo blando conducía dentro del rango y distorsionaba la señal ([simulations/README.md](../../simulations/README.md), sección 1).
- **A revisar** según el ADC: si el tiempo de adquisición del RP2350B es corto, puede hacer falta un capacitor de reserva (por ejemplo 1–10 nF C0G) y reducir la resistencia. Ese capacitor agrega un polo que tiene que ser igual en los dos canales.

## 13. Presupuesto de error a 50 Hz

| Fuente | Ganancia | Fase | Tratamiento |
|---|---|---|---|
| Relación del TC | ±3 % espec. (+1,5 % típ. medido) | — | Calibración |
| Magnetización del TC (RB = 15 Ω) | −0,2 % | +3,8° (varía con la corriente) | Calibración de fase, idealmente según la corriente |
| Burden 0,1 % | ±0,1 % | — | — |
| VBIAS (47 Ω) | −0,47 % | ≈ 0° | Calibración |
| Relación del potenciómetro (INL) | Hasta ±12,5 % con D = 8 | — | Calibración por código |
| Ganancia de lazo finita (G/AOL) | 32/100 000 = 0,03 % | — | Despreciable |
| Ancho de banda de U1 | — | −0,13° con G = 32 | Calibración por código |
| Filtro RF de entrada | — | −0,02° | Despreciable |
| Filtro MFB (R2/R1 al 1 %) | ±2 % | −1,5° (igual en los dos canales) | Calibración; se cancela con el canal de tensión |

Simulación con el modelo completo (ver [simulations/README.md](../../simulations/README.md)): **−0,02 dB y +2,2° a +2,3° a 50 Hz para todas las ganancias**, coherente con la suma de la tabla.

**Calibración necesaria:** ganancia y fase por cada código de ganancia usado (6 códigos), con carga resistiva (FP = 1) y, si es posible, a varias corrientes.

## 14. Lista de componentes del canal

| Ref. (simulación) | Valor | Tipo / tolerancia | Función |
|---|---|---|---|
| RB | 15 Ω | 1206, 0,25 W, 0,1 %, ≤ 25 ppm/°C | Burden, junto al conector |
| RIN | 1 kΩ | 1 % | Filtro RF y limitación de corriente |
| CIN | 1 nF | C0G | Filtro RF (a VBIAS) |
| U1, U2, U3 | LM358 | — | Ganancia, filtro, buffer de VBIAS |
| XPOT | MCP41010 | — | Ganancia variable (divisor) |
| R1, R2 | 10 kΩ | 1 % | Filtro MFB |
| R3 | 3,6 kΩ | 1 % | Filtro MFB |
| C1 | 22 nF | C0G, 2 % (5 % mínimo) | Filtro MFB |
| C2 | 4,7 nF | C0G, 2 % (5 % mínimo) | Filtro MFB |
| RPDG, RPDF, RPDB | 1 kΩ | 1 % | Pull-downs contra distorsión de cruce |
| RBT, RBB | 10 kΩ | 1 % | Divisor de VBIAS |
| CBD | 1 µF (recomendado 10 µF) | X7R alcanza | Filtrado del divisor |
| RBI | 47 Ω | 1 % | Aislación del seguidor |
| CBI | 1 µF | X7R alcanza | Desacople de VBIAS |
| RADC | 1 kΩ | 1 % | Protección del ADC |
| D5 | BAT54S | SOT-23 | Clamp a GND y 3,3 V |

Se eliminan del esquemático actual: R1 (1 MΩ), C2, R7, R11, C7, C12, R3, R5, C4 y el detector de pico de corriente (C13, D2, R15, R16, R18, R20, C17, C20, D4, J9). Estas referencias son las del esquemático original.

Después se volvió a agregar un detector de pico pasivo de media onda, tomado directo del secundario y referido a VBIAS: D9, R31–R34, C36, C37, D10 y la salida IPK, que va por J2 al MCP3204 de la placa digital. Cálculo y resultados en [calculos_front_end.md](calculos_front_end.md), sección 8. En el esquemático actual las resistencias se renumeraron; la tabla de arriba usa los nombres de la simulación. El canal de tensión usa ahora la misma etapa de ganancia (U5A + U7, MCP41010), así que los dos canales son idénticos desde la etapa de ganancia en adelante.

## 15. Puntos abiertos para revisar

1. **Frecuencia de muestreo:** con 10 240 Hz el filtro atenúa 21 dB lo que se pliega sobre la banda. Muestrear a 20 480 Hz y diezmar lo lleva a 34 dB (ver [calculos_front_end.md](calculos_front_end.md), sección 6.7).
2. **Estabilidad del seguidor de VBIAS** con 47 Ω + 1 µF, y si se puede bajar el error de 0,47 %. Requiere el modelo del fabricante o una prueba en banco.
3. **Máximo absoluto de los terminales del MCP41010** con la salida de U1 a 3,5 V en sobrecarga.
4. **Parámetros reales del TC** (Rw, fase): medirlos y actualizar el modelo de simulación.
5. **Canal de tensión:** tiene que usar el mismo filtro MFB (mismos valores y tipos de componentes) para que los desfasajes se cancelen.
6. **LM358 contra un operacional rail-to-rail a 3,3 V** (MCP6024, 10 MHz) para una segunda versión: amplía la excursión, baja el offset y elimina la distorsión de cruce y la variación de fase con la ganancia.
