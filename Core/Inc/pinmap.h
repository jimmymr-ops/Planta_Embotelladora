/*
 * pinmap.h
 *
 *  Created on: May 14, 2025
 *      Author: jaime
 */

#ifndef INC_PINMAP_H_
#define INC_PINMAP_H_

#include "stm32f4xx_hal.h"

/*Definimos los puertos que vamos a utilizar*/
#define INTERRUP_PORT    GPIOA               /*Puerto para las interrupciones*/
#define MOTOR_PORT       GPIOA                 /*... controlar los motores DC*/
#define DISP_LED_PORT    GPIOD                    /*... los LED y el disp7seg*/
#define INPUT_PORT       GPIOC                             /*... las entradas*/
#define OUTPUT_PORT      GPIOE                /*... las salidas incluye servo*/

 /*Definimos los pines que vamos a utilizar*/
#define PIN_LED_V   GPIO_PIN_12                 /*Pines para los led internos*/
#define PIN_LED_N   GPIO_PIN_13
#define PIN_LED_R   GPIO_PIN_14
#define PIN_LED_A   GPIO_PIN_15

#define DISP_SEG_A   GPIO_PIN_0                      /*Pines para el disp7seg*/
#define DISP_SEG_B   GPIO_PIN_1
#define DISP_SEG_C   GPIO_PIN_2
#define DISP_SEG_D   GPIO_PIN_3
#define DISP_SEG_E   GPIO_PIN_4
#define DISP_SEG_F   GPIO_PIN_5
#define DISP_SEG_G   GPIO_PIN_6

#define PIN_SENSOR_1     GPIO_PIN_2               /*Sensor de la taponadora 1*/
#define PIN_SENSOR_2     GPIO_PIN_3                                  /*.....2*/
#define PIN_SENSOR_3     GPIO_PIN_4                                  /*.....3*/
#define PIN_SENSOR_4     GPIO_PIN_5                                  /*.....4*/
#define PIN_SENSOR_5     GPIO_PIN_6                  /*Sensor cuenta botellas*/

#define PIN_SERVO        GPIO_PIN_9                   /*Pin con capacidad PWM*/
#define PIN_MOTOR_DC1    GPIO_PIN_8            /*Cinta transportadora llenado*/
#define PIN_MOTOR_DC2    GPIO_PIN_9           /*Motor, carrusel de taponadora*/
#define PIN_MOTOR_DC3    GPIO_PIN_10       /*Cinta transportadora empaquetado*/

#define PIN_VALVULA      GPIO_PIN_11                     /*Valvula de llenado*/
#define PIN_TAPON_1      GPIO_PIN_12                           /*taponadora 1*/
#define PIN_TAPON_2      GPIO_PIN_13                                 /*.....2*/
#define PIN_TAPON_3      GPIO_PIN_14                                 /*.....3*/
#define PIN_TAPON_4      GPIO_PIN_15                                 /*.....4*/

#define PIN_EXTI0        GPIO_PIN_0           /*Boton azul, sensor de llenado*/
#define PIN_EXTI1        GPIO_PIN_1       /*Interruptor paradas de emergencia*/




#endif /* INC_PINMAP_H_ */
