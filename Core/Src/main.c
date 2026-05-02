/* USER CODE BEGIN Header */
/**
 ******************************************************************************
 * @file           : main.c
 * @brief          : Main program body
 ******************************************************************************
 * @attention
 *
 * Copyright (c) 2026 STMicroelectronics.
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
#include "spi.h"
#include "tim.h"
#include "usart.h"
#include "gpio.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include <stdbool.h>
//#include <st7735.h>
//#include <fonts.h>
//#include <GFX_FUNCTIONS.h>
#include <string.h>
#include "adpcm.h"
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
#define BUFFER_LEN  20
#define DEFAULT_STARTUP_VAL (0x80)

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/

/* USER CODE BEGIN PV */
const unsigned char Ringtone[ADPCMD_PI];
int32_t pwm_val = 0;
bool direction = 0;
void LoadAudioFiles(void);

uint8_t RX_BUFFER[BUFFER_LEN];
uint8_t TX_BUFFER[BUFFER_LEN];
uint8_t rx_index;
uint8_t tx = 'O';
uint8_t text_request = 0;

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
/* USER CODE BEGIN PFP */
/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */
AudioElement AudioFile;
uint8_t AudioFileToPlay = 0;
/*
 void arrange_buffer(UART_HandleTypeDef huart, uint8_t* buffer, uint8_t buffer_length) {

 }
 */

void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart) {
	if (huart->Instance == USART1) {
		text_request = 1;
		HAL_UART_Receive_IT(&huart1, RX_BUFFER, 8);
		//RX_BUFFER[sizeof(RX_BUFFER)-1] = '\0';
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
  MX_SPI2_Init();
  MX_USART1_UART_Init();
  MX_TIM2_Init();
  /* USER CODE BEGIN 2 */

	//HAL_TIM_PWM_Start(&htim2, TIM_CHANNEL_1);
	//HAL_UART_Receive_IT(&huart1, RX_BUFFER, 2);
	//ST7735_Init(0);
	//fillScreen(BLACK);
	LoadAudioFiles();
  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
	/*
#ifdef USE_DAC
	//HAL_DAC_Start(&hdac1, DAC_CHANNEL_1);
	//HAL_DAC_SetValue(&hdac1, DAC_CHANNEL_1, DAC_ALIGN_12B_R, 0x7FF);
#endif
	// capture/compare registers (CC1 PWM duty 50%)
	TIM3->CCR1 = DEFAULT_STARTUP_VAL;
	TIM3->CCR2 = DEFAULT_STARTUP_VAL;
	//LL_TIM_EnableIT_UPDATE(TIM3);
	TIM3->CCER |= TIM_CCER_CC2E | TIM_CCER_CC1E;
	//LL_TIM_EnableCounter(TIM3);
	 *
	 */
	while (1) {

		// servo motor
		 TIM2->CCR1 = 1000;
		 HAL_Delay(1000);

		 TIM2->CCR1 = 1500;
		 HAL_Delay(1000);

		 TIM2->CCR1 = 2000;
		 HAL_Delay(1000);


		/*
		 HAL_UART_Transmit(&huart1, (uint8_t*) "AT\r\n", 4, 100);
		 HAL_Delay(3000);

		 HAL_UART_Transmit(&huart1, (uint8_t*) "AT+PSWD=cooki123\r\n", 16, 100);
		 ST7735_SetRotation(3);
		 ST7735_WriteString(0, 0, "Tried to change password", Font_11x18, CYAN,
		 BLACK);
		 HAL_Delay(3000);
		 fillScreen(BLACK);

		 HAL_UART_Transmit(&huart1, (uint8_t*) "AT+PSWD?\r\n", 8, 100);
		 HAL_Delay(3000);

		 if (text_request == 1) {
		 text_request = 0;

		 HAL_GPIO_TogglePin(GPIOA, GPIO_PIN_11);
		 HAL_Delay(500);
		 HAL_GPIO_TogglePin(GPIOA, GPIO_PIN_11);

		 ST7735_SetRotation(3);
		 ST7735_WriteString(0, 0, (char*) RX_BUFFER, Font_11x18, GREEN,
		 BLACK);
		 HAL_Delay(5000);
		 fillScreen(BLACK);

		 }*/

    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */

		/*
		 if (AudioFileToPlay >= 1) {
		 AudioFileToPlay = 0;
		 } else {
		 AudioFileToPlay++;
		 }
		 // Disable the TIM3 Interrupt
		 NVIC_EnableIRQ(TIM3_IRQn);
		 // stop the timer
		 LL_TIM_EnableCounter(TIM3);
		 HAL_Delay(100);
		 */

	}
  /* USER CODE END 3 */
}

/**
  * @brief System Clock Configuration
  * @retval None
  */
void SystemClock_Config(void)
{
  LL_FLASH_SetLatency(LL_FLASH_LATENCY_2);
  while(LL_FLASH_GetLatency()!= LL_FLASH_LATENCY_2)
  {
  }
  LL_RCC_HSE_Enable();

   /* Wait till HSE is ready */
  while(LL_RCC_HSE_IsReady() != 1)
  {

  }
  LL_RCC_PLL_ConfigDomain_SYS(LL_RCC_PLLSOURCE_HSE, LL_RCC_PLL_MUL_8, LL_RCC_PREDIV_DIV_1);
  LL_RCC_PLL_Enable();

   /* Wait till PLL is ready */
  while(LL_RCC_PLL_IsReady() != 1)
  {

  }
  LL_RCC_SetAHBPrescaler(LL_RCC_SYSCLK_DIV_1);
  LL_RCC_SetAPB1Prescaler(LL_RCC_APB1_DIV_2);
  LL_RCC_SetAPB2Prescaler(LL_RCC_APB2_DIV_1);
  LL_RCC_SetSysClkSource(LL_RCC_SYS_CLKSOURCE_PLL);

   /* Wait till System clock is ready */
  while(LL_RCC_GetSysClkSource() != LL_RCC_SYS_CLKSOURCE_STATUS_PLL)
  {

  }
  LL_SetSystemCoreClock(64000000);

   /* Update the time base */
  if (HAL_InitTick (TICK_INT_PRIORITY) != HAL_OK)
  {
    Error_Handler();
  }
  LL_RCC_SetUSARTClockSource(LL_RCC_USART1_CLKSOURCE_PCLK2);
  LL_RCC_SetTIMClockSource(LL_RCC_TIM2_CLKSOURCE_PCLK1);
}

/* USER CODE BEGIN 4 */
void LoadAudioFiles(void) {
	AudioFile.AudioFiles[0] = (uint32_t) &Ringtone;
	AudioFile.AudioSize[0] = NELEMS(Ringtone);
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
	while (1) {
	}
  /* USER CODE END Error_Handler_Debug */
}
#ifdef USE_FULL_ASSERT
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
