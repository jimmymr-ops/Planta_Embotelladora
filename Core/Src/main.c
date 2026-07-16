/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2025 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
/* USER CODE END Header */
/* Includes ------------------------------------------------------------------*/
#include "main.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "bottler.h"
#include "pinmap.h"
#include <stdio.h>
#include <stdbool.h>
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
#define IniciarTemporizadorEmbotelladora HAL_TIM_Base_Start_IT(&htim2)
#define IniciarTemporizadorTaponadora    HAL_TIM_Base_Start_IT(&htim3)
#define IniciarTemporizadorEmpaquetadora HAL_TIM_Base_Start_IT(&htim4)
#define DetenerTemporizadorEmbotelladora HAL_TIM_Base_Stop(&htim2);
#define DetenerTemporizadorTaponadora    HAL_TIM_Base_Stop(&htim3);
#define DetenerTemporizadorEmpaquetadora HAL_TIM_Base_Stop(&htim4);

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
TIM_HandleTypeDef htim1;
TIM_HandleTypeDef htim2;
TIM_HandleTypeDef htim3;
TIM_HandleTypeDef htim4;

/* USER CODE BEGIN PV */

enum Estados {
    BOTELLA_DETECTADA,
    AVANZA,
    ESPERA,
    TEMPORIZADOR_TERMINADO,
    PARADA_EMERGENCIA
};

static volatile enum Estados Embotelladora = ESPERA;                   /*Todas empiezan en espera*/
static volatile enum Estados Taponadora = ESPERA;
static volatile enum Estados Empaquetadora = ESPERA;
static volatile enum Estados EmbotelladoraAnterior = ESPERA;     /*Variables creadas para guardar*/
static volatile enum Estados TaponadoraAnterior = ESPERA;      /*El estado anterior de la maquina*/
static volatile enum Estados EmpaquetadoraAnterior = ESPERA;
static volatile bool paradaEmergencia;                                      /*flag activar parada*/
static volatile bool pulsado = false;                                      /*flag pulsador parada*/
static volatile bool reanudar = false;                                   /*flag reactivar maquina*/
static volatile uint8_t taponadoraActual = 0;
static volatile uint8_t contadorBotellas = 0;
static volatile uint8_t contador1 = 0;
static volatile uint8_t contador2 = 0;

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_TIM2_Init(void);
static void MX_TIM4_Init(void);
static void MX_TIM1_Init(void);
static void MX_TIM3_Init(void);
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{

  /* USER CODE BEGIN 1 */

  /* USER CODE END 1 */

  /* MCU Configuration--------------------------------------------------------*/

  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
  HAL_Init();

  /* USER CODE BEGIN Init */

  /* USER CODE END Init */

  /* Configure the system clock */
  SystemClock_Config();

  /* USER CODE BEGIN SysInit */

  /* USER CODE END SysInit */

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  MX_TIM2_Init();
  MX_TIM4_Init();
  MX_TIM1_Init();
  MX_TIM3_Init();
  /* USER CODE BEGIN 2 */
  GPIO_CONFIG();
  bottlerInit();
  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
	  if(paradaEmergencia){
		  NVIC_DisableIRQ(EXTI0_IRQn);               /*Deshabilitamos las interrupciones externas*/
		  NVIC_DisableIRQ(EXTI2_IRQn);
		  NVIC_DisableIRQ(EXTI3_IRQn);
		  NVIC_DisableIRQ(EXTI4_IRQn);
		  NVIC_DisableIRQ(EXTI9_5_IRQn);

		  DetenerTemporizadorEmbotelladora;    /*Detenemos los temporizadores guardando la cuenta*/
		  DetenerTemporizadorTaponadora;
		  DetenerTemporizadorEmpaquetadora;

	      EmbotelladoraAnterior = Embotelladora;   /*Guardamos el estado anterior de las maquinas*/
	      TaponadoraAnterior = Taponadora;
	      EmpaquetadoraAnterior = Empaquetadora;
	      Embotelladora = PARADA_EMERGENCIA;           /*Activamos el estado parada de emergencia*/
	      Taponadora = PARADA_EMERGENCIA;
	      Empaquetadora = PARADA_EMERGENCIA;
	      EncenderLedRojo;                                       /*Led señalizador de emergencias*/
	      ApagarLedVerde;
	      NVIC_EnableIRQ(EXTI1_IRQn);
	      paradaEmergencia = false;                                        /*Actualizamos la flag*/
	  }
	  if(reanudar){
	      Embotelladora = EmbotelladoraAnterior;                  /*Devolvemos al estado anterior*/
	      Taponadora = TaponadoraAnterior;
	      Empaquetadora = EmpaquetadoraAnterior;
	      if(EmbotelladoraAnterior == ESPERA) {              /*Si estaba en espera la reactivamos*/
	    	  MotorEmbotelladoraON;
	      }
	      MotorTaponadoraON;                                 /*Reactiva la cinta transportadora 2*/
	      EncenderLedVerde;                                      /*Led señalizador de funcionando*/
	      ApagarLedRojo;
	      reanudar = false;

		  NVIC_EnableIRQ(EXTI0_IRQn);                 /*Rehabilitamos las interrupciones externas*/
		  NVIC_EnableIRQ(EXTI2_IRQn);
		  NVIC_EnableIRQ(EXTI3_IRQn);
		  NVIC_EnableIRQ(EXTI4_IRQn);
		  NVIC_EnableIRQ(EXTI9_5_IRQn);

		  IniciarTemporizadorEmbotelladora;           /*Reactiva los relojes sin perder la cuenta*/
		  IniciarTemporizadorTaponadora;
		  IniciarTemporizadorEmpaquetadora;
	  }
	  switch(Embotelladora){                 /*Maquina de estados que controla a la embotelladora*/
	  case BOTELLA_DETECTADA:
	  	  MotorEmbotelladoraOFF;
	  	  AbrirValvula;
	  	  EncenderLedAzul;
		  Embotelladora = ESPERA;
		  IniciarTemporizadorEmbotelladora;                /*Activa el temporizador de 3 segundos*/
	  	  break;
	  case TEMPORIZADOR_TERMINADO:
	  	  MotorEmbotelladoraON;
	  	  CerrarValvula;
	  	  ApagarLedAzul;
	  	  Embotelladora = ESPERA;
	  	  break;
	  case ESPERA:
	  	  break;
	  case PARADA_EMERGENCIA:
	  	  MotorEmbotelladoraOFF;
	  	  CerrarValvula;
	  	  break;
	  default:                                            /*AVANZA solo aplica a la empaquetadora*/
		  break;
      }
	  switch(Taponadora){                       /*Maquina de estados que controla a la taponadora*/
	  case BOTELLA_DETECTADA:
		  taponBajar(taponadoraActual);                    /*Activa la correspondiente taponadora*/
		  IniciarTemporizadorTaponadora;                  /*Activa una cuenta atras de 2 segundos*/
		  EncenderLedNaranja;
		  Taponadora = ESPERA;
		  break;
	  case TEMPORIZADOR_TERMINADO:
		  taponSubir(taponadoraActual);              /*Desactivar la taponadora tras dos segundos*/
		  ApagarLedNaranja;
		  Taponadora = ESPERA;
		  break;
	  case ESPERA:
		  break;
	  case PARADA_EMERGENCIA:
		  MotorTaponadoraOFF;
		  break;
	  default:                                            /*AVANZA solo aplica a la empaquetadora*/
		  break;
	  }
	  switch(Empaquetadora){                 /*Maquina de estados que controla a la empaquetadora*/
	  case BOTELLA_DETECTADA:
		  servoAlternate();                                  /*se coloca una botella en cada lado*/
		  if(contadorBotellas%2 == 0) {                       /*cada dos botellas la cinta avanza*/
			  Empaquetadora = AVANZA;
		  }
		  else{Empaquetadora = ESPERA;}          /*solo una botella en la fila, se pone en espera*/
		  contadorBotellas++;
		  if(contadorBotellas>6){contadorBotellas = 1;}    /*cada 6 botellas es una caja completa*/
		  controlDisplay(contadorBotellas);                     /*Mostramos el numero de botellas*/
		  mydispShow();                                                 /*En el display7segmentos*/
		  break;
	  case AVANZA:
		  MotorEmpaquetadoraON;                  /*la cinta avanza para colocar dos huecos libres*/
		  IniciarTemporizadorEmpaquetadora;                           /*temporizador de 1 segundo*/
		  Empaquetadora = ESPERA;
		  break;
	  case ESPERA:
		  break;
	  case TEMPORIZADOR_TERMINADO:
		  MotorEmpaquetadoraOFF;                     /*Detiene la cinta para colocar mas botellas*/
		  Empaquetadora = ESPERA;
		  break;
	  case PARADA_EMERGENCIA:
		  MotorEmpaquetadoraOFF;
		  break;
	  }

    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
  }
  /* USER CODE END 3 */
}

/**
  * @brief System Clock Configuration
  * @retval None
  */
void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

  /** Configure the main internal regulator output voltage
  */
  __HAL_RCC_PWR_CLK_ENABLE();
  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE1);

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_ON;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLM = 4;
  RCC_OscInitStruct.PLL.PLLN = 150;
  RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV2;
  RCC_OscInitStruct.PLL.PLLQ = 7;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV4;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV2;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_4) != HAL_OK)
  {
    Error_Handler();
  }
}

/**
  * @brief TIM1 Initialization Function
  * @param None
  * @retval None
  */
static void MX_TIM1_Init(void)
{

  /* USER CODE BEGIN TIM1_Init 0 */

  /* USER CODE END TIM1_Init 0 */

  TIM_ClockConfigTypeDef sClockSourceConfig = {0};
  TIM_MasterConfigTypeDef sMasterConfig = {0};
  TIM_OC_InitTypeDef sConfigOC = {0};
  TIM_BreakDeadTimeConfigTypeDef sBreakDeadTimeConfig = {0};

  /* USER CODE BEGIN TIM1_Init 1 */

  /* USER CODE END TIM1_Init 1 */
  htim1.Instance = TIM1;
  htim1.Init.Prescaler = 1499;
  htim1.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim1.Init.Period = 1999;
  htim1.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
  htim1.Init.RepetitionCounter = 0;
  htim1.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_ENABLE;
  if (HAL_TIM_Base_Init(&htim1) != HAL_OK)
  {
    Error_Handler();
  }
  sClockSourceConfig.ClockSource = TIM_CLOCKSOURCE_INTERNAL;
  if (HAL_TIM_ConfigClockSource(&htim1, &sClockSourceConfig) != HAL_OK)
  {
    Error_Handler();
  }
  if (HAL_TIM_PWM_Init(&htim1) != HAL_OK)
  {
    Error_Handler();
  }
  sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
  sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
  if (HAL_TIMEx_MasterConfigSynchronization(&htim1, &sMasterConfig) != HAL_OK)
  {
    Error_Handler();
  }
  sConfigOC.OCMode = TIM_OCMODE_PWM1;
  sConfigOC.Pulse = 0;
  sConfigOC.OCPolarity = TIM_OCPOLARITY_HIGH;
  sConfigOC.OCNPolarity = TIM_OCNPOLARITY_HIGH;
  sConfigOC.OCFastMode = TIM_OCFAST_DISABLE;
  sConfigOC.OCIdleState = TIM_OCIDLESTATE_RESET;
  sConfigOC.OCNIdleState = TIM_OCNIDLESTATE_RESET;
  if (HAL_TIM_PWM_ConfigChannel(&htim1, &sConfigOC, TIM_CHANNEL_1) != HAL_OK)
  {
    Error_Handler();
  }
  sBreakDeadTimeConfig.OffStateRunMode = TIM_OSSR_DISABLE;
  sBreakDeadTimeConfig.OffStateIDLEMode = TIM_OSSI_DISABLE;
  sBreakDeadTimeConfig.LockLevel = TIM_LOCKLEVEL_OFF;
  sBreakDeadTimeConfig.DeadTime = 0;
  sBreakDeadTimeConfig.BreakState = TIM_BREAK_DISABLE;
  sBreakDeadTimeConfig.BreakPolarity = TIM_BREAKPOLARITY_HIGH;
  sBreakDeadTimeConfig.AutomaticOutput = TIM_AUTOMATICOUTPUT_DISABLE;
  if (HAL_TIMEx_ConfigBreakDeadTime(&htim1, &sBreakDeadTimeConfig) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN TIM1_Init 2 */

  /* USER CODE END TIM1_Init 2 */
  HAL_TIM_MspPostInit(&htim1);

}

/**
  * @brief TIM2 Initialization Function
  * @param None
  * @retval None
  */
static void MX_TIM2_Init(void)
{

  /* USER CODE BEGIN TIM2_Init 0 */

  /* USER CODE END TIM2_Init 0 */

  TIM_ClockConfigTypeDef sClockSourceConfig = {0};
  TIM_MasterConfigTypeDef sMasterConfig = {0};

  /* USER CODE BEGIN TIM2_Init 1 */

  /* USER CODE END TIM2_Init 1 */
  htim2.Instance = TIM2;
  htim2.Init.Prescaler = 7499;
  htim2.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim2.Init.Period = 9999;
  htim2.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
  htim2.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
  if (HAL_TIM_Base_Init(&htim2) != HAL_OK)
  {
    Error_Handler();
  }
  sClockSourceConfig.ClockSource = TIM_CLOCKSOURCE_INTERNAL;
  if (HAL_TIM_ConfigClockSource(&htim2, &sClockSourceConfig) != HAL_OK)
  {
    Error_Handler();
  }
  sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
  sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
  if (HAL_TIMEx_MasterConfigSynchronization(&htim2, &sMasterConfig) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN TIM2_Init 2 */

  /* USER CODE END TIM2_Init 2 */

}

/**
  * @brief TIM3 Initialization Function
  * @param None
  * @retval None
  */
static void MX_TIM3_Init(void)
{

  /* USER CODE BEGIN TIM3_Init 0 */

  /* USER CODE END TIM3_Init 0 */

  TIM_ClockConfigTypeDef sClockSourceConfig = {0};
  TIM_MasterConfigTypeDef sMasterConfig = {0};

  /* USER CODE BEGIN TIM3_Init 1 */

  /* USER CODE END TIM3_Init 1 */
  htim3.Instance = TIM3;
  htim3.Init.Prescaler = 7499;
  htim3.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim3.Init.Period = 9999;
  htim3.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
  htim3.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_ENABLE;
  if (HAL_TIM_Base_Init(&htim3) != HAL_OK)
  {
    Error_Handler();
  }
  sClockSourceConfig.ClockSource = TIM_CLOCKSOURCE_INTERNAL;
  if (HAL_TIM_ConfigClockSource(&htim3, &sClockSourceConfig) != HAL_OK)
  {
    Error_Handler();
  }
  sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
  sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
  if (HAL_TIMEx_MasterConfigSynchronization(&htim3, &sMasterConfig) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN TIM3_Init 2 */

  /* USER CODE END TIM3_Init 2 */

}

/**
  * @brief TIM4 Initialization Function
  * @param None
  * @retval None
  */
static void MX_TIM4_Init(void)
{

  /* USER CODE BEGIN TIM4_Init 0 */

  /* USER CODE END TIM4_Init 0 */

  TIM_ClockConfigTypeDef sClockSourceConfig = {0};
  TIM_MasterConfigTypeDef sMasterConfig = {0};

  /* USER CODE BEGIN TIM4_Init 1 */

  /* USER CODE END TIM4_Init 1 */
  htim4.Instance = TIM4;
  htim4.Init.Prescaler = 7499;
  htim4.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim4.Init.Period = 9999;
  htim4.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
  htim4.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
  if (HAL_TIM_Base_Init(&htim4) != HAL_OK)
  {
    Error_Handler();
  }
  sClockSourceConfig.ClockSource = TIM_CLOCKSOURCE_INTERNAL;
  if (HAL_TIM_ConfigClockSource(&htim4, &sClockSourceConfig) != HAL_OK)
  {
    Error_Handler();
  }
  sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
  sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
  if (HAL_TIMEx_MasterConfigSynchronization(&htim4, &sMasterConfig) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN TIM4_Init 2 */

  /* USER CODE END TIM4_Init 2 */

}

/**
  * @brief GPIO Initialization Function
  * @param None
  * @retval None
  */
static void MX_GPIO_Init(void)
{
  GPIO_InitTypeDef GPIO_InitStruct = {0};
/* USER CODE BEGIN MX_GPIO_Init_1 */
/* USER CODE END MX_GPIO_Init_1 */

  /* GPIO Ports Clock Enable */
  __HAL_RCC_GPIOC_CLK_ENABLE();
  __HAL_RCC_GPIOH_CLK_ENABLE();
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();
  __HAL_RCC_GPIOE_CLK_ENABLE();

  /*Configure GPIO pin : BOOT1_Pin */
  GPIO_InitStruct.Pin = BOOT1_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(BOOT1_GPIO_Port, &GPIO_InitStruct);

/* USER CODE BEGIN MX_GPIO_Init_2 */
/* USER CODE END MX_GPIO_Init_2 */
}

/* USER CODE BEGIN 4 */
void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin){        /*Interrupciones de los sensores y botones*/
    switch(GPIO_Pin){

    case PIN_EXTI0:                                                 /*interrupcion del boton azul*/
        Embotelladora = BOTELLA_DETECTADA;                                 /*detector de botellas*/
    	break;
    case PIN_EXTI1:                                 /*Interrupcion del boton parada de emergencia*/
    	if(!pulsado) {                                              /*Primera pulsación detectada*/
    	   paradaEmergencia = true;                              /*activamos la flag de la parada*/
    	   pulsado = true;                                     /*Registramos la segunda pulsación*/
    	}
    	else {                                                      /*Segunda pulsación detectada*/
    	   reanudar = true;                   /*Se activa la flag para reanudar el funcionamiento*/
    	   pulsado = false;                              /*Se reinicia el contador de pulsaciones*/
    	}
    	break;
    case PIN_SENSOR_1:                        /*Interrupciones para los sensores de la taponadora*/
    	Taponadora = BOTELLA_DETECTADA;                   /*Cambian el estado a botella detectada*/
    	taponadoraActual = 1;                    /*Indica que taponadora ha detectado una botella*/
    	break;
    case PIN_SENSOR_2:
    	Taponadora = BOTELLA_DETECTADA;
    	taponadoraActual = 2;
    	break;
    case PIN_SENSOR_3:
    	Taponadora = BOTELLA_DETECTADA;
    	taponadoraActual = 3;
    	break;
    case PIN_SENSOR_4:
    	Taponadora = BOTELLA_DETECTADA;
    	taponadoraActual = 4;
    	break;
    case PIN_SENSOR_5:                          /*Interrupcion para el sensor de la empaquetadora*/
    	Empaquetadora = BOTELLA_DETECTADA;                /*Cambian el estado a botella detectada*/
    	break;
    }
}
void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim) {
    if (htim == &htim2) {
    	contador1++;
    	if(contador1==3){                            /*Cuenta de 3 segundo para cerrar la valvula*/
    	Embotelladora = TEMPORIZADOR_TERMINADO;             /*Comunica que la cuenta ha terminado*/
    	contador1 = 0;
    	DetenerTemporizadorEmbotelladora;               /*Se desactiva hasta ser llamado de nuevo*/
    	}
    }
    else if (htim == &htim3) {
    	contador2++;                                       /*Cuenta de 2 segundo para poner tapon*/
    	if(contador2==2){
    	Taponadora = TEMPORIZADOR_TERMINADO;
    	contador2=0;
    	DetenerTemporizadorTaponadora;
    	}
    }
    else if (htim == &htim4) {                         /*Cuenta de 1 segundo para mover las cajas*/
    	Empaquetadora = TEMPORIZADOR_TERMINADO;             /*El primer update ya es el 1er segundo*/
    	DetenerTemporizadorEmpaquetadora;
    }
}
/* USER CODE END 4 */

/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler_Debug */
  /* User can add his own implementation to report the HAL error return state */
  __disable_irq();
  while (1)
  {
  }
  /* USER CODE END Error_Handler_Debug */
}

#ifdef  USE_FULL_ASSERT
/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t *file, uint32_t line)
{
  /* USER CODE BEGIN 6 */
  /* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */
