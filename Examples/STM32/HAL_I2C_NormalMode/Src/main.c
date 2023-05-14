/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  *
  * This is an example of use BME280_driver with STM32 platform and HAL library.
  * There is one sensor connected to I2C1, pin SDO is connected to GND
  * Results are stored as integers inside bme1_data structure.
  * Sensor if initialized, then configured to work with normal
  * mode. Last measured value is read every 500ms inside infinite loop.
  *
  ******************************************************************************
  */
/* USER CODE END Header */
/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "i2c.h"
#include "gpio.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "bme280.h"
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

/* USER CODE BEGIN PV */
BME280_t bme1;	// this is the structure of single BME280 sensor used in this example

BME280_Driver_t bme1_driver;	// this is the driver structure where read/write/delay functions and hi2c1 pointer

BME280_Config_t bme1_config;	// this is config structure where we should set all sensor parameters like filter etc.

BME280_Data_t bme1_data;		// this is result data structure, all meaures will be stored here
/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
/* USER CODE BEGIN PFP */
/* here are declarations of driver's functions specific for STM32 */
int8_t bme280_read_platform_spec(uint8_t reg_addr, uint8_t *rxbuff, uint8_t rxlen, void *driver);
int8_t bme280_write_platform_spec(uint8_t reg_addr, uint8_t value, void *driver);
void bme280_delay_platform_spec(uint8_t delay_time);
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
int8_t res;
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
  MX_I2C1_Init();
  /* USER CODE BEGIN 2 */

  /* fill the driver */
  bme1_driver.read = bme280_read_platform_spec;
  bme1_driver.write = bme280_write_platform_spec;
  bme1_driver.delay = bme280_delay_platform_spec;
  bme1_driver.env_spec_data = &hi2c1;
  bme1_driver.i2c_address = BME280_I2CADDR_SDOL;

  /* initialize sensor */
  res = BME280_Init(&bme1, &bme1_driver);
  if(BME280_OK != res){

	  //do something when error occured
  }

  /* sensor's options */
  bme1_config.oversampling_h = BME280_OVERSAMPLING_X4;
  bme1_config.oversampling_p = BME280_OVERSAMPLING_X2;
  bme1_config.oversampling_t = BME280_OVERSAMPLING_X2;
  bme1_config.filter = BME280_FILTER_2;
  bme1_config.spi3w_enable = 0;
  bme1_config.t_stby = BME280_STBY_500MS;
  bme1_config.mode = BME280_NORMALMODE;

  /* set all sensor's options */
  BME280_ConfigureAll(&bme1, &bme1_config);
  if(BME280_OK != res){

	  //do something when error occured
  }

  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {

	  /* force single read and return all data as integers */
	  res = BME280_ReadAllLast(&bme1, &bme1_data);
	  if(BME280_OK != res){

		  //do something when error occured
	  }

	  /* wait 500ms */
	  HAL_Delay(500);

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

  /** Configure LSE Drive Capability
  */
  HAL_PWR_EnableBkUpAccess();

  /** Configure the main internal regulator output voltage
  */
  __HAL_RCC_PWR_CLK_ENABLE();
  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE3);

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_BYPASS;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLM = 4;
  RCC_OscInitStruct.PLL.PLLN = 96;
  RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV2;
  RCC_OscInitStruct.PLL.PLLQ = 4;
  RCC_OscInitStruct.PLL.PLLR = 2;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  /** Activate the Over-Drive mode
  */
  if (HAL_PWREx_EnableOverDrive() != HAL_OK)
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

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_3) != HAL_OK)
  {
    Error_Handler();
  }
}

/* USER CODE BEGIN 4 */
int8_t bme280_read_platform_spec(uint8_t reg_addr, uint8_t *rxbuff, uint8_t rxlen, void *driver){

	/* check parameters */
	if((NULL == rxbuff) || (NULL == driver)) return -1;

	/* prepare local pointer */
	BME280_Driver_t *drv = (BME280_Driver_t *)driver;

	/* read data from sensor (refer to HAL documentation for details) */
	if(HAL_I2C_Mem_Read((I2C_HandleTypeDef *)drv->env_spec_data, (drv->i2c_address << 1),
			reg_addr, 1, rxbuff, rxlen, 200) != HAL_OK) return -1;

	return 0;
}

int8_t bme280_write_platform_spec(uint8_t reg_addr, uint8_t value, void *driver){

	/* check parameter */
	if(NULL == driver) return -1;

	/* prepare local pointers */
	BME280_Driver_t *drv = (BME280_Driver_t *)driver;

	/* write data to sensor (refer to HAL documentation for details) */
	if(HAL_I2C_Mem_Write((I2C_HandleTypeDef *)drv->env_spec_data, (drv->i2c_address << 1),
			reg_addr, 1, &value, 1, 200) != HAL_OK) return -1;

	return 0;
}

void bme280_delay_platform_spec(uint8_t delay_time){

	HAL_Delay(delay_time);
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
