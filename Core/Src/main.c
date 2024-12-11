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
#include "usart.h"
#include "gpio.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include <stdio.h>
#include "visualizer.h"
#include "pwm-fan-speed.h"
#include "huffman-compression.h"
#include "simple_random.h"
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
#define PUTCHAR_PROTOTYPE int __io_putchar(int ch)
/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/

/* USER CODE BEGIN PV */
char data[32];
double total_ms;
/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);

/* USER CODE BEGIN PFP */

double bench(void (*benchmark)(double*), uint32_t input_len,
    uint32_t iter, uint32_t rescale);
double bench_int(void (*benchmark)(unsigned int*), uint32_t input_len,
    uint32_t iter, uint32_t rescale_mod, int32_t rescale_offset);
/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

/* USER CODE END 0 */

/**
 * @brief  The application entry point.
 * @retval int
 */
int main(void) {

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
  MX_USART2_UART_Init();
  /* USER CODE BEGIN 2 */

  // Enable the counter
  DWT->CTRL |= DWT_CTRL_CYCCNTENA_Msk;
  CoreDebug->DEMCR |= CoreDebug_DEMCR_TRCENA_Msk;
  // Set random seed
  random_set_seed(42);
  uint32_t iters = 100;

  // Visualizer
  total_ms = bench(visualizer, VIS_INPUT_SIZE, iters, 100);
  printf("Visualizer, %lu iterations: %.4f ms\r\n", iters, total_ms);
  // Pwm-fan-speed
  total_ms = bench(pwm_fan_speed, PWM_INPUT_SIZE, iters, 5);
  printf("Pwm fan speed controller, %lu iterations: %.4f ms\r\n", iters,
      total_ms);
  // Huffman compression
  total_ms = bench_int(huffman_compression, HUFFMAN_INPUT_SIZE, iters, 95,
      32);
  printf("Huffman compression, %lu iterations: %.4f ms\r\n", iters,
      total_ms);

  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1) {
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
  }
  /* USER CODE END 3 */
}

/**
 * @brief System Clock Configuration
 * @retval None
 */
void SystemClock_Config(void) {
  RCC_OscInitTypeDef RCC_OscInitStruct = { 0 };
  RCC_ClkInitTypeDef RCC_ClkInitStruct = { 0 };

  /** Configure the main internal regulator output voltage
   */
  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE1);

  /** Initializes the RCC Oscillators according to the specified parameters
   * in the RCC_OscInitTypeDef structure.
   */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSI;
  RCC_OscInitStruct.PLL.PLLMUL = RCC_PLL_MUL6;
  RCC_OscInitStruct.PLL.PLLDIV = RCC_PLL_DIV3;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK) {
    Error_Handler();
  }

  /** Initializes the CPU, AHB and APB buses clocks
   */
  RCC_ClkInitStruct.ClockType =
  RCC_CLOCKTYPE_HCLK | RCC_CLOCKTYPE_SYSCLK | RCC_CLOCKTYPE_PCLK1
      | RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV1;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_1)
      != HAL_OK) {
    Error_Handler();
  }
}

/* USER CODE BEGIN 4 */

/**
 * @brief Runs the given benchmark.
 *
 * @param benchmark: the main benchmark function
 * @param input_len: the length of the input for the benchmark
 * @param iter: the number of iterations: each iteration will have different input
 * @param rescale: a parameter to rescale the input (which is in the range U[0,1)
 * @return double: the duration of the benchmark, in ms
 */
double bench(void (*benchmark)(double*), uint32_t input_len,
    uint32_t iter, uint32_t rescale) {
  double input[input_len];
  double total_cycles = 0;
  for (int i = 0; i < iter; ++i) {
    // Randomize array
    random_get_array(input, input_len);
    // Optionally, rescale the input
    for (int i = 0; i < input_len; ++i) {
      input[i] *= rescale;
    }
    // Reset the system counter to avoid overflows
    DWT->CYCCNT = 0;
    // Run the bench
    benchmark(input);
    // Get the total number of cycles
    total_cycles += (DWT->CYCCNT);
  }
  // Convert from cycles to milliseconds
  return (total_cycles * 1000.0 / HAL_RCC_GetSysClockFreq());
}

/**
 * @brief Runs the given benchmark, with integer inputs.
 *        A rescaling on the input is done as follows:
 *          input[i] = input[i] % rescale_mod + rescale_offset
 *        thus allowing for arbitrary integer values.
 *
 * @param benchmark: the main benchmark function
 * @param input_len: the length of the input for the benchmark
 * @param iter: the number of iterations: each iteration will have different input
 * @param rescale_mod: the range of integer values (from 0 to rescale_mod-1)
 * @param rescale_offset: the range offset (to sum to each input value)
 * @return double: the duration of the benchmark, in ms
 */
double bench_int(void (*benchmark)(unsigned int*), uint32_t input_len,
    uint32_t iter, uint32_t rescale_mod, int32_t rescale_offset) {
  uint32_t input[input_len];
  double total_cycles = 0;
  for (int i = 0; i < iter; ++i) {
    // Randomize array
    random_get_iarray(input, input_len);
    // Optionally, rescale the input
    for (int i = 0; i < input_len; ++i) {
      input[i] = input[i] % rescale_mod + rescale_offset;
    }
    // Reset the system counter to avoid overflows
    DWT->CYCCNT = 0;
    // Run the bench
    benchmark((unsigned int*)input);
    // Get the total number of cycles
    total_cycles += (DWT->CYCCNT);
  }
  // Convert from cycles to milliseconds
  return (total_cycles * 1000.0 / HAL_RCC_GetSysClockFreq());
}

PUTCHAR_PROTOTYPE {
  if (HAL_UART_Transmit(&huart2, (uint8_t*) &ch, 1, 0xFFFF) != HAL_OK) {
    Error_Handler();
  }

  return (ch);
}

/* USER CODE END 4 */

/**
 * @brief  This function is executed in case of error occurrence.
 * @retval None
 */
void Error_Handler(void) {
  /* USER CODE BEGIN Error_Handler_Debug */
  /* User can add his own implementation to report the HAL error return state */
  __disable_irq();
  while (1) {
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
