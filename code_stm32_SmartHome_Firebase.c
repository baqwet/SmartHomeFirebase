/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2024 STMicroelectronics.
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
#include <stdio.h>
#include <string.h>
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
ADC_HandleTypeDef hadc1;

TIM_HandleTypeDef htim2;

UART_HandleTypeDef huart1;

/* USER CODE BEGIN PV */

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_ADC1_Init(void);
static void MX_USART1_UART_Init(void);
static void MX_TIM2_Init(void);
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

int gas1=0, gas2=0, gas3=0;
int dad=0;
uint16_t adcVal=0;
char str[30];
uint8_t rx_buff[30];
uint8_t rev, i=0;
int ck=0;

void delay_1us() { // 4.5us
	__HAL_TIM_SetCounter(&htim2, 0);
    uint32_t ticks = 0;//(SystemCoreClock / 1000000);
    __HAL_TIM_SET_AUTORELOAD(&htim2, ticks);

    HAL_TIM_Base_Start(&htim2);
    while (__HAL_TIM_GET_FLAG(&htim2, TIM_FLAG_UPDATE) == RESET){}
	HAL_TIM_Base_Stop(&htim2);
}
//void delay_1ms(void)
//{
//	for(int t=0;t<1000;t++)
//		delay_1us();
//}
void delay_us(uint16_t time)
{
 while(time--)
 {
   delay_1us();
 }
}
//void delay_ms(uint16_t time)
//{
// while(time--)
// {
//   delay_1ms();
// }
//}

uint16_t readADC(int channel)
{
	ADC_ChannelConfTypeDef sConfigPrivate = {0};
	uint16_t u16_ADCVal=0;

	sConfigPrivate.Rank = ADC_REGULAR_RANK_1;
	if (channel==1)
	{
		sConfigPrivate.Channel = ADC_CHANNEL_0;
	}
	else if (channel==2)
	{
		sConfigPrivate.Channel = ADC_CHANNEL_1;
	}
	else if (channel==3)
	{
		sConfigPrivate.Channel = ADC_CHANNEL_2;
	}
	else
	{
		sConfigPrivate.Channel = ADC_CHANNEL_3;
	}
	sConfigPrivate.SamplingTime = ADC_SAMPLETIME_1CYCLE_5;
	HAL_ADC_ConfigChannel(&hadc1, &sConfigPrivate);
	HAL_ADC_Start(&hadc1);
	HAL_ADC_PollForConversion(&hadc1, 1000);
	u16_ADCVal = HAL_ADC_GetValue(&hadc1);
	HAL_ADC_Stop(&hadc1);

	return u16_ADCVal;
}
int16_t map(int16_t x, int16_t in_min, int16_t in_max, int16_t out_min, int16_t out_max) {
  return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
}
void CloseDOOR()
{
	int x = 320;
	for(int j=0;j<20;j++)
	{
		HAL_GPIO_WritePin(Servo3_GPIO_Port, Servo3_Pin, 1);
		delay_us( x);
		HAL_GPIO_WritePin(Servo3_GPIO_Port, Servo3_Pin, 0);
		delay_us(4444-x);
	}
}
void OpenDOOR()
{
	int x = 600;
	for(int j=0;j<20;j++)
	{
		HAL_GPIO_WritePin(Servo3_GPIO_Port, Servo3_Pin, 1);
		delay_us( x);
		HAL_GPIO_WritePin(Servo3_GPIO_Port, Servo3_Pin, 0);
		delay_us(4444-x);
	}
}
void CloseGATE()
{
	int x = 235;
	for(int j=0;j<20;j++)
	{
		HAL_GPIO_WritePin(Servo2_GPIO_Port, Servo2_Pin, 1);
		delay_us( x);
		HAL_GPIO_WritePin(Servo2_GPIO_Port, Servo2_Pin, 0);
		delay_us(4444-x);
	}

	x = 605;
	for(int j=0;j<20;j++)
	{
		HAL_GPIO_WritePin(Servo1_GPIO_Port, Servo1_Pin, 1);
		delay_us( x);
		HAL_GPIO_WritePin(Servo1_GPIO_Port, Servo1_Pin, 0);
		delay_us(4444-x);
	}
}
void OpenGATE()
{
	int x = 510;
	for(int j=0;j<20;j++)
	{
		HAL_GPIO_WritePin(Servo2_GPIO_Port, Servo2_Pin, 1);
		delay_us( x);
		HAL_GPIO_WritePin(Servo2_GPIO_Port, Servo2_Pin, 0);
		delay_us(4444-x);
	}

	x = 320;
	for(int j=0;j<20;j++)
	{
		HAL_GPIO_WritePin(Servo1_GPIO_Port, Servo1_Pin, 1);
		delay_us( x);
		HAL_GPIO_WritePin(Servo1_GPIO_Port, Servo1_Pin, 0);
		delay_us(4444-x);
	}
}
void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{
	if(huart->Instance == USART1){
		HAL_UART_Receive_IT(&huart1, &rev, 1);
		if (rev != '\n' && i<20)
		{
			rx_buff[i] = rev;
			i++;
		}
		else
		{
			if (strstr((char*)rx_buff, "CHECK") != NULL)
				ck=1;
			if (strstr((char*)rx_buff, "FanON") != NULL)
				HAL_GPIO_WritePin(Fan_GPIO_Port, Fan_Pin, 1);
			if (strstr((char*)rx_buff, "FanOFF") != NULL)
				HAL_GPIO_WritePin(Fan_GPIO_Port, Fan_Pin, 0);
			if (strstr((char*)rx_buff, "OpenDOOR") != NULL)
				OpenDOOR();
			if (strstr((char*)rx_buff, "CloseDOOR") != NULL)
				CloseDOOR();
			if (strstr((char*)rx_buff, "OpenGATE") != NULL)
				OpenGATE();
			if (strstr((char*)rx_buff, "CloseGATE") != NULL)
				CloseGATE();
			if (strstr((char*)rx_buff, "Led1ON") != NULL)
				HAL_GPIO_WritePin(Led1_GPIO_Port, Led1_Pin, 0);
			if (strstr((char*)rx_buff, "Led1OFF") != NULL)
				HAL_GPIO_WritePin(Led1_GPIO_Port, Led1_Pin, 1);
			if (strstr((char*)rx_buff, "Led2ON") != NULL)
				HAL_GPIO_WritePin(Led2_GPIO_Port, Led2_Pin, 0);
			if (strstr((char*)rx_buff, "Led2OFF") != NULL)
				HAL_GPIO_WritePin(Led2_GPIO_Port, Led2_Pin, 1);
			if (strstr((char*)rx_buff, "Led3ON") != NULL)
				HAL_GPIO_WritePin(Led3_GPIO_Port, Led3_Pin, 0);
			if (strstr((char*)rx_buff, "Led3OFF") != NULL)
				HAL_GPIO_WritePin(Led3_GPIO_Port, Led3_Pin, 1);
			i=0;
			memset(rx_buff,0,sizeof(rx_buff));
		}
	}
}

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
  MX_ADC1_Init();
  MX_USART1_UART_Init();
  MX_TIM2_Init();
  /* USER CODE BEGIN 2 */
//  HAL_TIM_Base_Start(&htim2);
  HAL_TIM_Base_Init(&htim2);
	memset(rx_buff,0,sizeof(rx_buff));
	HAL_UART_Receive_IT(&huart1, &rev, 1);

	CloseDOOR();
	CloseGATE();
	while (1)
	{
		adcVal = readADC(2);
		gas1 = (int) map(adcVal,700,3000,0,99);
		if (gas1<0) gas1=0;
		if (gas1>99) gas1=99;
		adcVal = readADC(3);
		gas2 = (int) map(adcVal,700,3000,0,99);
		if (gas2<0) gas2=0;
		if (gas2>99) gas2=99;
		adcVal = readADC(4);
		gas3 = (int) map(adcVal,700,3000,0,99);
		if (gas3<0) gas3=0;
		if (gas3>99) gas3=99;

		if (gas1<10 && gas2<10 && gas3<10) break;
	}
  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
	  dad = (int) map(readADC(1),3870,0,0,99);
	  if (dad<0) dad=0;
	  if (dad>99) dad=99;
	  if (dad<15) HAL_GPIO_WritePin(Pump_GPIO_Port, Pump_Pin, 1);
	  if (dad>20) HAL_GPIO_WritePin(Pump_GPIO_Port, Pump_Pin, 0);

	  adcVal = readADC(2);
	  gas1 = (int) map(adcVal,700,3000,0,99);
	  if (gas1<0) gas1=0;
	  if (gas1>99) gas1=99;

	  adcVal = readADC(3);
	  gas2 = (int) map(adcVal,700,3000,0,99);
	  if (gas2<0) gas2=0;
	  if (gas2>99) gas2=99;

	  adcVal = readADC(4);
	  gas3 = (int) map(adcVal,700,3000,0,99);
	  if (gas3<0) gas3=0;
	  if (gas3>99) gas3=99;

	  if (ck==1)
	  {
		  sprintf(str,"%2d,%2d,%2d\n",gas1,gas2,gas3);
		  HAL_UART_Transmit(&huart1, (uint8_t*)str, sizeof(str), 100);
		  HAL_Delay(50);
		  ck=0;
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
  RCC_PeriphCLKInitTypeDef PeriphClkInit = {0};

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_ON;
  RCC_OscInitStruct.HSEPredivValue = RCC_HSE_PREDIV_DIV1;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLMUL = RCC_PLL_MUL9;
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
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV2;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_2) != HAL_OK)
  {
    Error_Handler();
  }
  PeriphClkInit.PeriphClockSelection = RCC_PERIPHCLK_ADC;
  PeriphClkInit.AdcClockSelection = RCC_ADCPCLK2_DIV6;
  if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInit) != HAL_OK)
  {
    Error_Handler();
  }
}

/**
  * @brief ADC1 Initialization Function
  * @param None
  * @retval None
  */
static void MX_ADC1_Init(void)
{

  /* USER CODE BEGIN ADC1_Init 0 */

  /* USER CODE END ADC1_Init 0 */

  ADC_ChannelConfTypeDef sConfig = {0};

  /* USER CODE BEGIN ADC1_Init 1 */

  /* USER CODE END ADC1_Init 1 */

  /** Common config
  */
  hadc1.Instance = ADC1;
  hadc1.Init.ScanConvMode = ADC_SCAN_DISABLE;
  hadc1.Init.ContinuousConvMode = ENABLE;
  hadc1.Init.DiscontinuousConvMode = DISABLE;
  hadc1.Init.ExternalTrigConv = ADC_SOFTWARE_START;
  hadc1.Init.DataAlign = ADC_DATAALIGN_RIGHT;
  hadc1.Init.NbrOfConversion = 1;
  if (HAL_ADC_Init(&hadc1) != HAL_OK)
  {
    Error_Handler();
  }

  /** Configure Regular Channel
  */
  sConfig.Channel = ADC_CHANNEL_0;
  sConfig.Rank = ADC_REGULAR_RANK_1;
  sConfig.SamplingTime = ADC_SAMPLETIME_1CYCLE_5;
  if (HAL_ADC_ConfigChannel(&hadc1, &sConfig) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN ADC1_Init 2 */

  /* USER CODE END ADC1_Init 2 */

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
  htim2.Init.Prescaler = 72-1;
  htim2.Init.CounterMode = TIM_COUNTERMODE_DOWN;
  htim2.Init.Period = 0xffff;
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
  * @brief USART1 Initialization Function
  * @param None
  * @retval None
  */
static void MX_USART1_UART_Init(void)
{

  /* USER CODE BEGIN USART1_Init 0 */

  /* USER CODE END USART1_Init 0 */

  /* USER CODE BEGIN USART1_Init 1 */

  /* USER CODE END USART1_Init 1 */
  huart1.Instance = USART1;
  huart1.Init.BaudRate = 9600;
  huart1.Init.WordLength = UART_WORDLENGTH_8B;
  huart1.Init.StopBits = UART_STOPBITS_1;
  huart1.Init.Parity = UART_PARITY_NONE;
  huart1.Init.Mode = UART_MODE_TX_RX;
  huart1.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  huart1.Init.OverSampling = UART_OVERSAMPLING_16;
  if (HAL_UART_Init(&huart1) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN USART1_Init 2 */

  /* USER CODE END USART1_Init 2 */

}

/**
  * @brief GPIO Initialization Function
  * @param None
  * @retval None
  */
static void MX_GPIO_Init(void)
{
  GPIO_InitTypeDef GPIO_InitStruct = {0};

  /* GPIO Ports Clock Enable */
  __HAL_RCC_GPIOD_CLK_ENABLE();
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOB, Led1_Pin|Led2_Pin|Led3_Pin, GPIO_PIN_SET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOB, Servo1_Pin|Servo2_Pin|Servo3_Pin|Fan_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(Pump_GPIO_Port, Pump_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pins : Led1_Pin Led2_Pin Led3_Pin Servo1_Pin
                           Servo2_Pin Servo3_Pin Fan_Pin */
  GPIO_InitStruct.Pin = Led1_Pin|Led2_Pin|Led3_Pin|Servo1_Pin
                          |Servo2_Pin|Servo3_Pin|Fan_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

  /*Configure GPIO pin : Pump_Pin */
  GPIO_InitStruct.Pin = Pump_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(Pump_GPIO_Port, &GPIO_InitStruct);

}

/* USER CODE BEGIN 4 */

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
