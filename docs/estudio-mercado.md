# Estudio de Mercado y Diseño de Analizador Trifásico de Calidad de Energía

> Documento de investigación fuente. Sus conclusiones están incorporadas y desarrolladas en [`architecture.md`](architecture.md), que es el documento de referencia vivo del proyecto — este archivo se conserva como respaldo del análisis original.

Este documento compila el análisis de mercado, criterios de diseño de hardware, veta comercial orientada al mantenimiento predictivo e implementación técnica con **Zephyr RTOS** y el microcontrolador **Raspberry Pi RP2350B**.

---

## 1. Estado del Mercado Actual de Analizadores Trifásicos

El mercado de analizadores de redes está dominado por marcas de alta gama con características específicas orientadas a auditorías móviles:

*   **Fluke Serie 1770 (1773, 1775, 1777):** Destacan por su sistema de alimentación directa desde el circuito de medida y una interfaz táctil con corrección guiada digital de errores de conexión. El modelo 1777 captura transitorios de hasta 8 kV y armónicos hasta 30 kHz.
*   **Hioki PQ3198:** Reconocido por su alta precisión de laboratorio y cumplimiento estricto de la norma **IEC 61000-4-30 Clase A**, ideal para certificaciones legales.
*   **Megger MPQ1000:** Sobresale en la industria por su capacidad de analizar armónicos detallados hasta el orden 128º en un formato portátil.

### Tabla Comparativa de Equipos Comerciales

| Criterio / Modelo | Fluke 1777 | Hioki PQ3198 | Megger MPQ1000 |
| :--- | :--- | :--- | :--- |
| **Tipo de Formato** | Portátil con pantalla táctil | Portátil para auditorías | Portátil / Manual (Handheld) |
| **Rango de Armónicos** | Hasta 30 kHz | Hasta el orden 50º | Hasta el orden 128º |
| **Captura de Transitorios**| Sí, hasta 8 kV (1 MS/s) | Sí, de alta precisión | Sí, eventos de calidad de red |
| **Alimentación** | Directa desde el circuito medido | Adaptador AC / Batería | Adaptador AC / Batería |
| **Ventaja Principal** | Corrección digital de errores y sin cables de alimentación externa | Excelente balance entre precisión y almacenamiento a largo plazo | Máxima resolución armónica en formato compacto |

---

## 2. La Veta Comercial e Innovación para la Tesis de Grado

Los equipos comerciales tradicionales sufren de **tres barreras críticas**: precios prohibitivos para PYMEs, diseño pensado para uso itinerante (maletín) y ecosistemas de software cerrados.

Para una tesis de grado de bajo costo, la oportunidad de innovación (Océano Azul) no está en competir con la velocidad analógica de la Clase A, sino en la **contextualización de datos y el Mantenimiento Predictivo (Machine Health)** mediante el **Análisis de la Firma de Corriente del Motor (MCSA)**.

### Enfoque Seleccionado: Mantenimiento Predictivo para PYMEs
El equipo se diseña para una instalación fija en formato **Riel DIN** al lado de maquinaria crítica. No solo mide la calidad eléctrica básica, sino que procesa algoritmos ligeros para alertar sobre fallas mecánicas e internas del motor:
*   **Fallas en Rodamientos/Excentricidad:** Modulaciones en la FFT de la corriente alrededor de los 50 Hz ($50 \text{ Hz} \pm f_{vibracion}$).
*   **Desbalance Dinámico:** Monitoreo del calentamiento excesivo por desequilibrio eléctrico entre fases.

---

## 3. Arquitectura del Hardware de Adquisición y Frontend Universal

Para garantizar un equipo universal capaz de adaptarse a múltiples topologías eléctricas de forma autónoma, se implementa una arquitectura simétrica de **8 canales del ADC interno del RP2350B** (4 voltajes y 4 corrientes) combinada con un esquema de expansión para el control de ganancia.

### Configuración y Auto-detección de Red
*   **Conexiones Físicas:** El equipo dispone de 4 bornes de tensión ($L_1, L_2, L_3, N$) y 4 de corriente ($I_1, I_2, I_3, I_N$).
*   **Neutro Virtual de Hardware:** Los transformadores de voltaje (**ZMPT101B**) se referencian internamente a un punto estrella artificial mediante resistencias de alto valor ($1\text{ M}\Omega$). Esto permite conectar redes tanto en **Estrella (4 hilos)** como en **Triángulo (3 hilos)** sin usar switches físicos.
*   **Algoritmo de Detección (Firmware):** El RP2350B evalúa los desfases (secuencia de fases 120°/240°) y la actividad de la corriente/voltaje en el borne $N$. Si el neutro flota y no hay corriente de retorno, conmuta automáticamente a matemática de Triángulo (calculando voltajes de línea mediante $V_{L1-L2} = V_{1N} - V_{2N}$).

### Conectores y Seguridad Industrial
Se eligen **conectores tipo Jack de audio de 3.5 mm estéreo o conectores de aviación GX12** en lugar de bornes tipo banana. Esto asegura compatibilidad directa con pinzas de corriente de núcleo partido económicas (como la serie **SCT-013** con salida en voltaje 0-1V o 0-333mV) y evita que el secundario de las pinzas quede peligrosamente abierto bajo carga.

### Alimentación e Instrumentación de Respaldo
*   **Toma de Línea Trifásica:** Un puente rectificador trifásico toma energía de las 3 fases hacia una fuente conmutada (*Ultra-Wide Input*) conmutada. El equipo se autoalimenta de la línea sin importar si una o dos fases fallan.
*   **Batería Backup:** Se integra una celda Li-ion 18650 (3.7V) administrada por un chip con *Power Path Management* continuo. Si ocurre un apagón, un pin de interrupción avisa a Zephyr para apagar la pantalla, cerrar los archivos en la SD de forma segura y registrar la bitácora del corte.

---

## 4. Control Automático de Ganancia (AGC) Dinámico

Para maximizar los 12 bits de resolución del ADC interno del RP2350B y medir con precisión hasta el **30vo armónico (1500 Hz / 1800 Hz)**, se implementa un lazo de control dinámico.

```
[Señal de TV/Pinza] ──> [PGA MCP6S21 (Ganancia Variable)] ──> [ADC Interno RP2350B (Ondas)]
                                │
                                └──> [Detector de Picos] ──> [ADC Externo ADS1115 (Picos)]
```

### El Amplificador de Ganancia Programable (PGA) vs Potenciómetro Digital
Se selecciona el PGA **MCP6S21** (controlado por SPI) sobre potenciómetros digitales (como el MCP4251) debido a:
1.  **Ancho de Banda y Fase Estable:** El PGA mantiene una respuesta en frecuencia plana y un desfasaje constante en todos sus rangos, crucial para el análisis FFT preciso. Los potenciómetros introducen capacitancias parásitas de 50-100 pF que crean filtros pasabajos variables descalibrando los armónicos altos.
2.  **Sin Resistencia del Wiper:** Evita el error no lineal que induce el cursor de los potenciómetros digitales ($\sim 75 \,\Omega$).

### Monitoreo con el ADS1115 Externo (I2C)
Para no sacrificar ninguno de los 8 canales analógicos de alta velocidad del RP2350B, se añade un ADC externo **ADS1115** dedicado exclusivamente a leer las salidas DC de los **detectores de picos analógicos** de las corrientes. Como el cambio de pico es muy lento, la velocidad del bus I2C es perfecta.

### Lógica de Histéresis del AGC en Firmware
Para prevenir reconfiguraciones constantes (*chattering*), el firmware implementa umbrales de histéresis:
*   **Umbral de Saturación (> 3.0 V):** Reduce un escalón de ganancia en el PGA para evitar el recorte (*clipping*) en la siguiente captura.
*   **Umbral de Baja Resolución (< 1.0 V):** Incrementa un escalón de ganancia para aprovechar el rango dinámico del ADC.
*   **Congelamiento por Bloque:** La ganancia cambia **únicamente en el tiempo muerto** entre ráfagas de captura del buffer circular, garantizando que los bloques de 1024 o 2048 muestras analizados por la librería `CMSIS-DSP` posean una ganancia estrictamente constante.

---

## 5. Arquitectura de Firmware en Zephyr RTOS y LVGL

El software aprovecha el procesador de doble núcleo del RP2350B para segmentar de manera determinista las tareas críticas:

```
[Núcleo 0 - Alta Prioridad]  ──> Timer HW + Adquisición ADC (Round-Robin 6.4 kHz) -> Buffer Ping-Pong
[Núcleo 1 - Media Prioridad] ──> Procesamiento DSP (CMSIS-DSP) -> FFT, RMS, Desbalances
[Núcleo 0 - Baja Prioridad]  ──> LVGL UI Task (Pantalla 3.5" Paralela + Encoder) + Almacenamiento SD
[Núcleo 0/1 - Hilo AGC]      ──> Lectura Asíncrona ADS1115 I2C (Cada 200 ms) + Conmutación SPI PGA
```

### Organización de Datos en la Tarjeta SD
La información procesada se guarda utilizando la API de sistemas de archivos de Zephyr (FATFS) estructurada en tres archivos `.CSV` para facilitar su extracción y análisis:
1.  `HISTORICO.CSV`: Valores promedio cada 10-15 minutos (RMS Voltaje/Corriente, Potencias, THD) para análisis de tendencias de consumo.
2.  `ARMONICOS.CSV`: Matriz completa de las amplitudes del 1 al 30 de cada fase, permitiendo mapear la firma espectral de la máquina industrial.
3.  `EVENTOS.CSV`: Bitácora instantánea estampada en tiempo real mediante un chip RTC externo (ej. DS3231) con alertas de desbalances críticos o caídas de tensión.
