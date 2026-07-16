/*
 * bottler.c
 *
 *  Created on: May 22, 2025
 *      Author: jaime
 */
#include <stdio.h>
#include "main.h"
#include "bottler.h"
#include "pinmap.h"

#define IniciarDisplay controlDisplay(0);

extern TIM_HandleTypeDef htim1;                                                      /*Pulsos PWM*/

static uint16_t seleccionarMotor(uint8_t motor);                          /*Funciones secundarias*/
static uint16_t seleccionarTaponadora(uint8_t taponadora);
static uint16_t seleccionarLed(uint8_t led);

const uint8_t tabla7seg[7] = {0x3F, 0x18, 0x76, 0x7C, 0x59, 0x6D, 0x4F};

void conveyorForward(uint8_t motor){                              /*Activa el motor seleccionado*/
	uint16_t motorPin = seleccionarMotor(motor);
	if(motorPin != 0) {
		HAL_GPIO_WritePin(MOTOR_PORT , motorPin, GPIO_PIN_SET);
	}
	else{printf("Motor seleccionado no encontrado\r\n");}
}

void conveyorStop(uint8_t motor){                                /*Detiene el motor seleccionado*/
	uint16_t motorPin = seleccionarMotor(motor);
	if(motorPin != 0) {
		HAL_GPIO_WritePin(MOTOR_PORT , motorPin, GPIO_PIN_RESET);
	}
	else{printf("Motor seleccionado no encontrado\r\n");}
}

void valveOpen(void){                                                           /*Abre la valvula*/
	HAL_GPIO_WritePin(OUTPUT_PORT, PIN_VALVULA, GPIO_PIN_SET);
}

void valveClose(void){                                                        /*Cierra la valvula*/
	HAL_GPIO_WritePin(OUTPUT_PORT, PIN_VALVULA, GPIO_PIN_RESET);
}

void servoAlternate(void){                                        /*Alterna la posicion del servo*/
    static uint8_t posicion = 0;

    if (posicion == 0) {                     /*El Tim1 esta configurado para tener ciclos de 20ms*/
        __HAL_TIM_SET_COMPARE(&htim1, TIM_CHANNEL_1, 50);                           /*0° = 0.5 ms*/
        posicion = 1;
    } else {
        __HAL_TIM_SET_COMPARE(&htim1, TIM_CHANNEL_1, 150);                         /*90° = 1.5 ms*/
        posicion = 0;
    }
}

void taponBajar(uint8_t taponadora){                          /*Activa la taponadora seleccionada*/
    uint16_t taponadoraPin = seleccionarTaponadora(taponadora);
    if (taponadoraPin != 0 ) {
        HAL_GPIO_WritePin(OUTPUT_PORT, taponadoraPin, GPIO_PIN_SET);
    }
    else {printf("Taponadora seleccionada no encontrada\r\n");}
}

void taponSubir(uint8_t taponadora){                       /*Desactiva la taponadora seleccionada*/
    uint16_t taponadoraPin = seleccionarTaponadora(taponadora);
    if (taponadoraPin != 0 ) {
        HAL_GPIO_WritePin(OUTPUT_PORT, taponadoraPin, GPIO_PIN_RESET);
    }
    else {printf("Taponadora seleccionada no encontrada\r\n");}
}

void encenderLed(uint8_t led) {                                    /*Enciende el led seleccionado*/
	uint16_t ledPin = seleccionarLed(led);
	if(ledPin != 0) {
		HAL_GPIO_WritePin(DISP_LED_PORT, ledPin, GPIO_PIN_SET);
	}
	else {printf("LED seleccionado no encontrado\r\n");}
}

void apagarLed(uint8_t led) {                                         /*Apaga el led seleccionado*/
	uint16_t ledPin = seleccionarLed(led);
	if(ledPin != 0) {
		HAL_GPIO_WritePin(DISP_LED_PORT, ledPin, GPIO_PIN_RESET);
	}
	else {printf("LED seleccionado no encontrado\r\n");}
}

void controlDisplay(uint8_t numero) {                                       /*Gestiona el display*/
	if(numero >= sizeof(tabla7seg)) {                        /*la tabla solo cubre los digitos 0-6*/
		printf("Numero fuera del rango del display\r\n");
		return;
	}
	DISP_LED_PORT -> BSRR = 0x7F << 16;                               /*Apaga todos los segmentos*/
	DISP_LED_PORT -> BSRR = (tabla7seg[numero]);                        /*Enciende los necesarios*/
}

void bottlerInit(void){                            /*Configuracion de inicio para los perifericos*/
	EncenderLedVerde;
	ApagarLedNaranja;
	ApagarLedRojo;
	ApagarLedAzul;
	MotorEmbotelladoraON;
	AbrirValvula;
	MotorTaponadoraON;
	IniciarDisplay;
	mydispShow();
	HAL_TIM_PWM_Start(&htim1, TIM_CHANNEL_1);      /*Arranca el contador y la salida PWM del servo*/
	HAL_NVIC_SetPriority(TIM2_IRQn, 0, 0);                        /*Habilitamos la interrupciones*/
	HAL_NVIC_EnableIRQ(TIM2_IRQn);                                  /*De lo tres relojes externos*/
	HAL_NVIC_SetPriority(TIM3_IRQn, 0, 0);
	HAL_NVIC_EnableIRQ(TIM3_IRQn);
    HAL_NVIC_SetPriority(TIM4_IRQn, 0, 0);
	HAL_NVIC_EnableIRQ(TIM4_IRQn);
}

static uint16_t seleccionarMotor(uint8_t motor){                    /*recibe un valor entre (1-3)*/
	switch(motor){                                               /*selecciona el pin de ese motor*/
	case 1:
		return PIN_MOTOR_DC1;
	case 2:
		return PIN_MOTOR_DC2;
	case 3:
		return PIN_MOTOR_DC3;
	default:
		return 0;
	}
}

static uint16_t seleccionarTaponadora(uint8_t taponadora){
	switch(taponadora){                                             /*recibe un valor entre (1-4)*/
	case 1:                                                /* selecciona el pin de esa taponadora*/
		return PIN_TAPON_1;
	case 2:
		return PIN_TAPON_2;
	case 3:
		return PIN_TAPON_3;
	case 4:
		return PIN_TAPON_4;
	default:
		return 0;
	}
}

static uint16_t seleccionarLed(uint8_t led) {
	switch(led){                                     /*Recibe la letra correspondiente a cada led*/
	case 'V':                                                                   /*devuelve su pin*/
		return PIN_LED_V;
	case 'N':
		return PIN_LED_N;
	case 'R':
		return PIN_LED_R;
	case 'A':
		return PIN_LED_A;
	default:
		return 0;
	}
}


