# Distribución de pines

Asignación completa de pines del STM32F407VGTx, tabla del display de 7 segmentos y cálculo de
los temporizadores. La fuente única de verdad en código es
[`Core/Inc/pinmap.h`](../Core/Inc/pinmap.h); este documento la acompaña con el detalle de
puertos libres, modos y cálculos que no caben en el header.

## Puertos

| Puerto | Macro en `pinmap.h` | Uso |
|---|---|---|
| A | `INTERRUP_PORT` / `MOTOR_PORT` | Interrupciones de entrada y motores DC |
| B | — | Sin usar (salvo `BOOT1`, fijado por hardware) |
| C | `INPUT_PORT` | Sensores de taponado y de recuento |
| D | `DISP_LED_PORT` | Display de 7 segmentos y LED de estado |
| E | `OUTPUT_PORT` | Servo, válvula y actuadores de taponado |
| F, G, H, I | — | Sin usar |

## Puerto A — `INTERRUP_PORT` / `MOTOR_PORT`

| Pin | Modo | Nombre | Periférico |
|---|---|---|---|
| `PA0` | `IT_RISING` | `EXTI0` | Botón azul (sensor de llenado) |
| `PA1` | `IT_RISING` | `EXTI1` | Botón de emergencia |
| `PA8` | `OUT_PP` | `MOTOR_DC1` | Motor 1 — cinta de llenado |
| `PA9` | `OUT_PP` | `MOTOR_DC2` | Motor 2 — carrusel de taponado |
| `PA10` | `OUT_PP` | `MOTOR_DC3` | Motor 3 — cinta de empaquetado |
| `PA13` | — | — | Debug (SWD) |
| `PA14` | — | — | Debug (SWD) |

Pines libres: `PA2`–`PA7`, `PA11`, `PA12`, `PA15`.

## Puerto B

| Pin | Modo | Pull | Periférico |
|---|---|---|---|
| `PB2` | `INPUT` | No | `BOOT1` |

El resto del puerto B está libre.

## Puerto C — `INPUT_PORT`

| Pin | Modo | Nombre | Periférico |
|---|---|---|---|
| `PC2` | `IT_RISING` | `SENSOR_1` | Sensor taponadora 1 |
| `PC3` | `IT_RISING` | `SENSOR_2` | Sensor taponadora 2 |
| `PC4` | `IT_RISING` | `SENSOR_3` | Sensor taponadora 3 |
| `PC5` | `IT_RISING` | `SENSOR_4` | Sensor taponadora 4 |
| `PC6` | `IT_RISING` | `SENSOR_5` | Sensor de recuento (empaquetado) |
| `PC14` | — | — | `OSC32_IN` |
| `PC15` | — | — | `OSC32_OUT` |

Todos los sensores llevan pull-down. Pines libres: `PC0`, `PC1`, `PC7`–`PC13`.

## Puerto D — `DISP_LED_PORT`

| Pin | Modo | Nombre | Periférico |
|---|---|---|---|
| `PD0` | `OUT_PP` | `DISP_SEG_A` | Display 7 segmentos |
| `PD1` | `OUT_PP` | `DISP_SEG_B` | Display 7 segmentos |
| `PD2` | `OUT_PP` | `DISP_SEG_C` | Display 7 segmentos |
| `PD3` | `OUT_PP` | `DISP_SEG_D` | Display 7 segmentos |
| `PD4` | `OUT_PP` | `DISP_SEG_E` | Display 7 segmentos |
| `PD5` | `OUT_PP` | `DISP_SEG_F` | Display 7 segmentos |
| `PD6` | `OUT_PP` | `DISP_SEG_G` | Display 7 segmentos |
| `PD12` | `OUT_PP` | `LED_V` | LED verde — en funcionamiento |
| `PD13` | `OUT_PP` | `LED_N` | LED naranja — taponando |
| `PD14` | `OUT_PP` | `LED_R` | LED rojo — parada de emergencia |
| `PD15` | `OUT_PP` | `LED_A` | LED azul — llenando |

Los segmentos ocupan `PD0`–`PD6` de forma contigua, lo que permite apagarlos de una sola
escritura atómica sobre `BSRR` (`0x7F << 16`). Pines libres: `PD7`–`PD11`.

## Puerto E — `OUTPUT_PORT`

| Pin | Modo | Nombre | Periférico |
|---|---|---|---|
| `PE9` | `AF1` / `TIM1_CH1` | `SERVO` | Servo repartidor (PWM) |
| `PE11` | `OUT_PP` | `VALVULA` | Válvula de llenado |
| `PE12` | `OUT_PP` | `TAPON_1` | Taponadora 1 |
| `PE13` | `OUT_PP` | `TAPON_2` | Taponadora 2 |
| `PE14` | `OUT_PP` | `TAPON_3` | Taponadora 3 |
| `PE15` | `OUT_PP` | `TAPON_4` | Taponadora 4 |

Pines libres: `PE0`–`PE8`, `PE10`.

## Temporizadores

Con el reloj de temporizadores de APB1 a **75 MHz**, un prescaler de `7499` deja el contador a
10 kHz y un período de `9999` completa un ciclo por segundo. Los tres temporizadores de proceso
generan por tanto **una interrupción por segundo**, y el número de segundos de cada etapa se
cuenta en `HAL_TIM_PeriodElapsedCallback()`.

| Timer | Prescaler | Período | Tiempo | Máquina |
|---|---|---|---|---|
| TIM2 | 7499 | 9999 | 3 s | Embotelladora |
| TIM3 | 7499 | 9999 | 2 s | Taponadora |
| TIM4 | 7499 | 9999 | 1 s | Empaquetadora |

El prescaler es de 16 bits; el período es de 32 bits en TIM2 y TIM5, y de 16 bits en el resto.

### TIM1 — PWM del servo

TIM1 cuelga de APB2 (150 MHz) y no forma parte de la tabla anterior: no cuenta segundos, sino
que genera la señal de 50 Hz (20 ms) del servo por su canal 1. Con el prescaler a `1499` el
contador queda a 100 kHz, de modo que **1 ms equivale a 100 unidades de `CCR1`** (1 unidad =
10 µs):

| Ángulo | Ancho de pulso | `CCR1` |
|---|---|---|
| 0° | 0,5 ms | 50 |
| 90° | 1,5 ms | 150 |

## Display de 7 segmentos

Ánodo común. La disposición física de los segmentos no sigue la nomenclatura estándar, sino
esta:

```
     ─── F ───
    │         │
    A         E
    │         │
     ─── G ───
    │         │
    B         D
    │         │
     ─── C ───
```

La tabla de conversión está indexada por dígito, con los bits ordenados `G F E D C B A`
(bit 0 = segmento A):

| G | F | E | D | C | B | A | Número | Binario | Hexa |
|---|---|---|---|---|---|---|---|---|---|
| 0 | 1 | 1 | 1 | 1 | 1 | 1 | 0 | `0b0111111` | `0x3F` |
| 0 | 0 | 1 | 1 | 0 | 0 | 0 | 1 | `0b0011000` | `0x18` |
| 1 | 1 | 1 | 0 | 1 | 1 | 0 | 2 | `0b1110110` | `0x76` |
| 1 | 1 | 1 | 1 | 1 | 0 | 0 | 3 | `0b1111100` | `0x7C` |
| 1 | 0 | 1 | 1 | 0 | 0 | 1 | 4 | `0b1011001` | `0x59` |
| 1 | 1 | 0 | 1 | 1 | 0 | 1 | 5 | `0b1101101` | `0x6D` |
| 1 | 0 | 0 | 1 | 1 | 1 | 1 | 6 | `0b1001111` | `0x4F` |
| 0 | 1 | 1 | 1 | 0 | 0 | 0 | 7 | `0b0111000` | `0x38` |
| 1 | 1 | 1 | 1 | 1 | 1 | 1 | 8 | `0b1111111` | `0x7F` |
| 1 | 1 | 1 | 1 | 0 | 0 | 1 | 9 | `0b1111001` | `0x79` |

> **Solo se implementan los dígitos 0–6.** La tabla `tabla7seg[]` de
> [`Core/Src/bottler.c`](../Core/Src/bottler.c) llega hasta el `0x4F` porque una caja son seis
> botellas; `controlDisplay()` rechaza cualquier valor superior. Los dígitos 7, 8 y 9 se
> calcularon durante el diseño y se conservan aquí por si el recuento cambia.

## Estados de los LED

| Estado | LED |
|---|---|
| En funcionamiento | Verde (`PD12`) |
| Llenando | Azul (`PD15`) |
| Taponando | Naranja (`PD13`) |
| Parada de emergencia | Rojo (`PD14`) |
