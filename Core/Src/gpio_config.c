/*
 * gpio_config.c
 *
 *  Created on: May 22, 2025
 *      Author: jaime
 */
#include "main.h"
#include "pinmap.h"

uint8_t disp7seg_init_passed = 0;

void GPIO_disp7seg_Config(void){                  /*Configuracion del display y los leds internos*/
    GPIO_InitTypeDef disp7seg = {0};

    disp7seg.Pin = DISP_SEG_A | DISP_SEG_B | DISP_SEG_C | DISP_SEG_D | DISP_SEG_E |
    		       DISP_SEG_F | DISP_SEG_G | PIN_LED_V | PIN_LED_N |PIN_LED_R | PIN_LED_A;
    disp7seg.Mode = GPIO_MODE_OUTPUT_PP;
    disp7seg.Pull = GPIO_NOPULL;
    disp7seg.Speed = GPIO_SPEED_FREQ_LOW;
    HAL_GPIO_Init(DISP_LED_PORT, &disp7seg);

	disp7seg_init_passed = 1;   /*mydisp.c comprueba que los pines del display esten configurados*/
}

void GPIO_Salidas_Config(void){                   /*Configuracion de la valvula y las taponadoras*/
	GPIO_InitTypeDef salidas = {0};

	salidas.Pin = PIN_VALVULA | PIN_TAPON_1 | PIN_TAPON_2 | PIN_TAPON_3 | PIN_TAPON_4 ;
	salidas.Mode = GPIO_MODE_OUTPUT_PP;
	salidas.Pull = GPIO_NOPULL;
	salidas.Speed = GPIO_SPEED_FREQ_LOW;

	HAL_GPIO_Init(OUTPUT_PORT, &salidas);
}

void GPIO_Motor_Config(void) {                          /*Configuracion de los tres motores DC*/
	GPIO_InitTypeDef motor = {0};

	motor.Pin = PIN_MOTOR_DC1 | PIN_MOTOR_DC2 | PIN_MOTOR_DC3;
	motor.Mode = GPIO_MODE_OUTPUT_PP;
	motor.Pull = GPIO_NOPULL;
	motor.Speed = GPIO_SPEED_FREQ_LOW;

	HAL_GPIO_Init(MOTOR_PORT, &motor);
}

void GPIO_Interrupciones_Config(void){    /*EXTI0-> BotonAzul == sensor, EXTI1 -> BotonEmergencia*/
	GPIO_InitTypeDef interrupcion = {0};

	interrupcion.Pin = PIN_EXTI0 | PIN_EXTI1;
	interrupcion.Mode = GPIO_MODE_IT_RISING;                     /*Detección de flancos de subida*/
	interrupcion.Pull = GPIO_PULLDOWN;

	HAL_GPIO_Init(INTERRUP_PORT, &interrupcion);

        HAL_NVIC_SetPriority(EXTI1_IRQn, 0, 0);         /*Mayor prioridad la parada de emergencia*/
        HAL_NVIC_EnableIRQ(EXTI1_IRQn);

        HAL_NVIC_SetPriority(EXTI0_IRQn, 1, 0);        /*Menor prioridad deteccion de una botella*/
        HAL_NVIC_EnableIRQ(EXTI0_IRQn);
}
void GPIO_Entradas_Config(void){         /*Configuracion de los Sensores(Interrupciones externas)*/
	GPIO_InitTypeDef entradas  = {0};

	entradas.Pin = PIN_SENSOR_1 | PIN_SENSOR_2 | PIN_SENSOR_3 | PIN_SENSOR_4 | PIN_SENSOR_5;
	entradas.Mode = GPIO_MODE_IT_RISING;
	entradas.Pull = GPIO_PULLDOWN;

	HAL_GPIO_Init(INPUT_PORT, &entradas);

	/*HAL_Init fija NVIC_PRIORITYGROUP_4: 4 bits de preempcion y 0 de subprioridad, por lo que el
	  tercer argumento se ignora y estos sensores quedan en preempcion 0, igual que la emergencia*/
	HAL_NVIC_SetPriority(EXTI2_IRQn, 0, 2);
	HAL_NVIC_EnableIRQ(EXTI2_IRQn);

	HAL_NVIC_SetPriority(EXTI3_IRQn, 0, 2);
 	HAL_NVIC_EnableIRQ(EXTI3_IRQn);

	HAL_NVIC_SetPriority(EXTI4_IRQn, 0, 2);
	HAL_NVIC_EnableIRQ(EXTI4_IRQn);

	HAL_NVIC_SetPriority(EXTI9_5_IRQn, 0, 2);
	HAL_NVIC_EnableIRQ(EXTI9_5_IRQn);
}

void GPIO_CONFIG(void){
	__HAL_RCC_GPIOA_CLK_ENABLE();       /*Se habilitan todos los puertos que van a ser utilizados*/
	__HAL_RCC_GPIOC_CLK_ENABLE();
	__HAL_RCC_GPIOD_CLK_ENABLE();
	__HAL_RCC_GPIOE_CLK_ENABLE();

    GPIO_disp7seg_Config();
	GPIO_Salidas_Config();
    GPIO_Motor_Config();
	GPIO_Interrupciones_Config();
	GPIO_Entradas_Config();
}



