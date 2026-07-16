# Planta embotelladora automatizada — STM32F407VG

Control de una planta embotelladora simulada, implementado sobre un STM32F407VGTx con
HAL de ST y desarrollado en STM32CubeIDE. El sistema coordina tres etapas independientes
(llenado, taponado y empaquetado) mediante tres máquinas de estados dirigidas por
interrupciones, con parada de emergencia y señalización por LED y display de 7 segmentos.

Proyecto de la asignatura **Informática Industrial II**.

## Descripción del proceso

La planta se divide en tres etapas que funcionan de forma concurrente, cada una gobernada
por su propia máquina de estados y su propio temporizador:

**1. Embotelladora.** Una cinta transportadora (`M1`) desplaza las botellas vacías. Cuando
el sensor de llenado detecta una botella bajo el surtidor, la cinta se detiene, se abre la
válvula y se enciende el LED azul. Tras **3 s** (TIM2) la válvula se cierra y la cinta
reanuda la marcha.

**2. Taponadora.** Un carrusel (`M2`) recibe las botellas llenas. Cuatro sensores
(`S1`–`S4`) vigilan cuatro puestos de taponado independientes. Al detectar una botella, baja
el tapón del puesto correspondiente y se enciende el LED naranja; tras **2 s** (TIM3) el
tapón sube.

**3. Empaquetadora.** Un quinto sensor (`S5`) cuenta las botellas terminadas. Un servo
alterna entre 0° y 90° para repartirlas en dos filas dentro de la caja; cada dos botellas la
cinta (`M3`) avanza **1 s** (TIM4) para dejar dos huecos libres. El display de 7 segmentos
muestra el recuento de la caja en curso, de 1 a 6 (una caja completa son **6 botellas**).

**Parada de emergencia.** Un pulsador dedicado, en la interrupción de mayor prioridad,
congela la planta: deshabilita el resto de interrupciones externas, detiene los tres
temporizadores *conservando su cuenta*, guarda el estado de cada máquina, apaga motores y
válvula y enciende el LED rojo. Una segunda pulsación restaura los estados guardados y
reanuda los temporizadores desde donde se quedaron.

## Esquema de la planta

| Documento | Contenido |
|---|---|
| [`docs/esquema-planta.pdf`](docs/esquema-planta.pdf) | Esquema final, correspondiente al código de este repositorio |
| [`docs/esquema-planta-preliminar.pdf`](docs/esquema-planta-preliminar.pdf) | Diseño previo, con multiplexores 74HC157 y otro reparto de pines |
| [`docs/distribucion-pines.xlsx`](docs/distribucion-pines.xlsx) | Asignación completa de pines, tabla del 7 segmentos y cálculo de temporizadores |

> **Nota sobre el esquema.** La leyenda del PDF final asigna los motores a `PA9`/`PA10`/`PA11`.
> El pinout real implementado —y el que recoge la hoja de cálculo— es `PA8`/`PA9`/`PA10`, tal
> y como aparece en la tabla de abajo y en `pinmap.h`. La válvula (`PE11`) y el servo (`PE9`)
> sí coinciden en las tres fuentes.

## Mapa de pines

Todas las asignaciones están centralizadas en [`Core/Inc/pinmap.h`](Core/Inc/pinmap.h).

### Entradas — interrupciones externas

| Pin | Modo | Nombre | Función |
|---|---|---|---|
| `PA0` | `IT_RISING`, pull-down | `PIN_EXTI0` | Sensor de llenado (botón azul) |
| `PA1` | `IT_RISING`, pull-down | `PIN_EXTI1` | Parada de emergencia |
| `PC2` | `IT_RISING`, pull-down | `PIN_SENSOR_1` | Sensor taponadora 1 |
| `PC3` | `IT_RISING`, pull-down | `PIN_SENSOR_2` | Sensor taponadora 2 |
| `PC4` | `IT_RISING`, pull-down | `PIN_SENSOR_3` | Sensor taponadora 3 |
| `PC5` | `IT_RISING`, pull-down | `PIN_SENSOR_4` | Sensor taponadora 4 |
| `PC6` | `IT_RISING`, pull-down | `PIN_SENSOR_5` | Cuenta botellas (empaquetado) |

### Salidas

| Pin | Modo | Nombre | Función |
|---|---|---|---|
| `PA8` | `OUT_PP` | `PIN_MOTOR_DC1` | Cinta transportadora de llenado |
| `PA9` | `OUT_PP` | `PIN_MOTOR_DC2` | Carrusel de la taponadora |
| `PA10` | `OUT_PP` | `PIN_MOTOR_DC3` | Cinta transportadora de empaquetado |
| `PE9` | `AF1` / `TIM1_CH1` | `PIN_SERVO` | Servo repartidor (PWM) |
| `PE11` | `OUT_PP` | `PIN_VALVULA` | Válvula de llenado |
| `PE12`–`PE15` | `OUT_PP` | `PIN_TAPON_1`–`4` | Actuadores de las cuatro taponadoras |
| `PD0`–`PD6` | `OUT_PP` | `DISP_SEG_A`–`G` | Display de 7 segmentos |
| `PD12` | `OUT_PP` | `PIN_LED_V` | LED verde — en funcionamiento |
| `PD13` | `OUT_PP` | `PIN_LED_N` | LED naranja — taponando |
| `PD14` | `OUT_PP` | `PIN_LED_R` | LED rojo — parada de emergencia |
| `PD15` | `OUT_PP` | `PIN_LED_A` | LED azul — llenando |

## Relojes y temporizadores

HSE de 8 MHz, PLL con `M=4`, `N=150`, `P=2` → **SYSCLK = 150 MHz**. APB1 a 37,5 MHz
(temporizadores a 75 MHz) y APB2 a 75 MHz (temporizadores a 150 MHz).

| Timer | Prescaler | Período | Frecuencia | Uso |
|---|---|---|---|---|
| TIM1 | 1499 | 1999 | 50 Hz (20 ms) | PWM del servo, canal 1 |
| TIM2 | 7499 | 9999 | 1 Hz | 3 s de llenado (embotelladora) |
| TIM3 | 7499 | 9999 | 1 Hz | 2 s de taponado |
| TIM4 | 7499 | 9999 | 1 Hz | 1 s de avance de cinta (empaquetado) |

TIM2, TIM3 y TIM4 generan una interrupción por segundo; el número de segundos de cada etapa
se cuenta en `HAL_TIM_PeriodElapsedCallback()`. Los tres se detienen con `HAL_TIM_Base_Stop()`
en lugar de `_Stop_IT()` de forma deliberada: así el contador conserva su valor y la parada de
emergencia puede reanudar la cuenta exactamente donde la dejó.

Con TIM1 a 100 kHz tras el prescaler, cada unidad de `CCR1` equivale a 10 µs: `50` → 0,5 ms (0°)
y `150` → 1,5 ms (90°).

## Display de 7 segmentos

`controlDisplay()` escribe directamente sobre el registro `BSRR` del puerto D: primero apaga
los siete segmentos en una sola operación atómica y después enciende los necesarios. La tabla
de conversión está indexada por dígito, con los bits ordenados `G F E D C B A` (bit 0 = segmento A):

| Dígito | Binario | Hex |
|---|---|---|
| 0 | `0b0111111` | `0x3F` |
| 1 | `0b0011000` | `0x18` |
| 2 | `0b1110110` | `0x76` |
| 3 | `0b1111100` | `0x7C` |
| 4 | `0b1011001` | `0x59` |
| 5 | `0b1101101` | `0x6D` |
| 6 | `0b1001111` | `0x4F` |

La tabla llega solo hasta el 6 porque una caja son 6 botellas. `mydisp.c` replica además el
display en ASCII por la traza SWO/ITM, útil para depurar sin mirar el esquema.

## Estructura del repositorio

```
Planta_Embotelladora/
├── Core/
│   ├── Inc/
│   │   ├── bottler.h        Interfaz de actuadores y macros de acciones fijas
│   │   ├── pinmap.h         Asignación de puertos y pines (fuente única de verdad)
│   │   └── main.h
│   ├── Src/
│   │   ├── main.c           Máquinas de estados, callbacks de EXTI y de timers
│   │   ├── bottler.c        Motores, válvula, servo, taponadoras, LED y display
│   │   ├── gpio_config.c    Configuración de GPIO y prioridades del NVIC
│   │   ├── mydisp.c         Réplica del 7 segmentos por SWO/ITM
│   │   └── stm32f4xx_it.c   Vectores de interrupción
│   └── Startup/
├── Drivers/                 HAL y CMSIS de ST
├── docs/                    Esquemas de la planta y distribución de pines
├── Planta_Embotelladora.ioc Configuración de STM32CubeMX
├── STM32F407VGTX_FLASH.ld   Linker script
└── STM32F407VGTX_RAM.ld
```

`main.c` concentra el control: las interrupciones solo actualizan estados y flags, y todo el
trabajo real (mover motores, abrir la válvula, refrescar el display) ocurre en el bucle
principal. Los actuadores se manejan a través de las macros de `bottler.h`
(`MotorEmbotelladoraON`, `AbrirValvula`, `EncenderLedRojo`…), de modo que el bucle principal
se lee como la descripción del proceso.

## Compilación

1. Clonar el repositorio.
2. Abrir STM32CubeIDE y elegir **File → Open Projects from File System…**
3. Seleccionar la carpeta del repositorio (es en sí misma el proyecto CubeIDE).
4. **Project → Build Project** (`Ctrl+B`)

Los drivers HAL están incluidos, así que el proyecto compila recién clonado sin necesidad de
regenerar nada desde CubeMX. El `.ioc` se incluye por si se quiere modificar la configuración
de periféricos.

Para ver la salida de `printf` (réplica del display y mensajes de error) hay que habilitar
**SWV ITM Data Console** en la configuración de depuración, con el núcleo a 150 MHz.

## Prioridades de interrupción

`HAL_Init()` fija `NVIC_PRIORITYGROUP_4`: **4 bits de preempción y 0 de subprioridad**, por lo
que el tercer argumento de `HAL_NVIC_SetPriority()` no tiene efecto alguno. Las prioridades
efectivas son:

| Interrupción | Preempción | Nota |
|---|---|---|
| `EXTI1` — emergencia | 0 | Máxima |
| `EXTI2/3/4/9_5` — sensores | 0 | Empatan con la emergencia; entre iguales no hay expropiación |
| `TIM2/3/4` | 0 | |
| `EXTI0` — sensor de llenado | 1 | La única que puede ser expropiada |

## Licencia

Los ficheros bajo `Drivers/` son propiedad de STMicroelectronics y se distribuyen bajo sus
propias licencias. El código de `Core/Src` y `Core/Inc` escrito para este proyecto se publica
con fines educativos.
