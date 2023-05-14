/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * File Name          : freertos.c
  * Description        : Code for freertos applications
  ******************************************************************************
  *
  * This is an example of use BME280_driver with STM32 platform and HAL library
  * placed in Freertos enviroment. Functions of this driver are not reentrant
  * so user is obligated to use mutexes to protect sensor's structure
  * and communication peripherials. Here we use only one sensor in one task
  * so mutex for BME280_t structure was skipped.
  * There is one sensor connected to SPI1 in 4-wire mode, chip select pin is defined
  * as BME280_NCS and driven by software.
  * Results are stored as integers inside bme1_data structure.
  * Sensor if initialized, then configured. Inside BME280_Task single
  * measure is forced every 500ms.
  *
  ******************************************************************************
  */
/* USER CODE END Header */

/* Includes ------------------------------------------------------------------*/
#include "FreeRTOS.h"
#include "task.h"
#include "main.h"
#include "cmsis_os.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "spi.h"
#include "semphr.h"
#include "bme280.h"
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */
struct spi_rtos {

	SPI_HandleTypeDef *spi_handle;	// handle to spi bus
	GPIO_TypeDef *NCS_gpio; 		// chip select gpio port
	uint16_t NCS_pin;				// chip select pin nr.
	SemaphoreHandle_t spi_mutex;
};
/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
/* USER CODE BEGIN Variables */

struct spi_rtos spi1_struct;	// this is the structure where pointer to spi structure and mutex will be stored

BME280_t bme1;					// this is the structure of single BME280 sensor used in this example
BME280_Driver_t bme1_driver;	// this is the driver structure for bme1 sensor (*spi1_struct + driver's functions)
BME280_Config_t bme1_config;	// this is config structure where we should set all sensor parameters like filter etc.
BME280_Data_t bme1_data;		// this is result data structure, all meaures will be stored here

/* USER CODE END Variables */
/* Definitions for BME280_Task */
osThreadId_t BME280_TaskHandle;
const osThreadAttr_t BME280_Task_attributes = {
  .name = "BME280_Task",
  .stack_size = 512 * 4,
  .priority = (osPriority_t) osPriorityNormal,
};

/* Private function prototypes -----------------------------------------------*/
/* USER CODE BEGIN FunctionPrototypes */
void bme1_init(void);

int8_t bme280_read_platform_spec(uint8_t reg_addr, uint8_t *rxbuff, uint8_t rxlen, void *driver);
int8_t bme280_write_platform_spec(uint8_t reg_addr, uint8_t value, void *driver);
void bme280_delay_platform_spec(uint8_t delay_time);
/* USER CODE END FunctionPrototypes */

void Start_BME280_Task(void *argument);

void MX_FREERTOS_Init(void); /* (MISRA C 2004 rule 8.1) */

/**
  * @brief  FreeRTOS initialization
  * @param  None
  * @retval None
  */
void MX_FREERTOS_Init(void) {
  /* USER CODE BEGIN Init */

	/* setup SPI1 structure */
	spi1_struct.spi_handle = &hspi1;
	spi1_struct.NCS_gpio = BME280_NCS_GPIO_Port;
	spi1_struct.NCS_pin = BME280_NCS_Pin;
	spi1_struct.spi_mutex = xSemaphoreCreateMutex();
	if(NULL == spi1_struct.spi_mutex){

		while(1){}
	}

  /* USER CODE END Init */

  /* USER CODE BEGIN RTOS_MUTEX */
  /* add mutexes, ... */
  /* USER CODE END RTOS_MUTEX */

  /* USER CODE BEGIN RTOS_SEMAPHORES */
  /* add semaphores, ... */
  /* USER CODE END RTOS_SEMAPHORES */

  /* USER CODE BEGIN RTOS_TIMERS */
  /* start timers, add new ones, ... */
  /* USER CODE END RTOS_TIMERS */

  /* USER CODE BEGIN RTOS_QUEUES */
  /* add queues, ... */
  /* USER CODE END RTOS_QUEUES */

  /* Create the thread(s) */
  /* creation of BME280_Task */
  BME280_TaskHandle = osThreadNew(Start_BME280_Task, NULL, &BME280_Task_attributes);

  /* USER CODE BEGIN RTOS_THREADS */
  /* add threads, ... */
  /* USER CODE END RTOS_THREADS */

  /* USER CODE BEGIN RTOS_EVENTS */
  /* add events, ... */
  /* USER CODE END RTOS_EVENTS */

}

/* USER CODE BEGIN Header_Start_BME280_Task */
/**
  * @brief  Function implementing the BME280_Task thread.
  * @param  argument: Not used
  * @retval None
  */
/* USER CODE END Header_Start_BME280_Task */
void Start_BME280_Task(void *argument)
{
  /* USER CODE BEGIN Start_BME280_Task */
	bme1_init();
  /* Infinite loop */
  for(;;){

	  BME280_ReadAllForce(&bme1, &bme1_data);
	  vTaskDelay(pdMS_TO_TICKS(500));
  }
  /* USER CODE END Start_BME280_Task */
}

/* Private application code --------------------------------------------------*/
/* USER CODE BEGIN Application */
void bme1_init(void){

	/* setup bme1 driver */
	bme1_driver.read = bme280_read_platform_spec;
	bme1_driver.write = bme280_write_platform_spec;
	bme1_driver.delay = bme280_delay_platform_spec;
	bme1_driver.env_spec_data = &spi1_struct;

	/* initialize sensor */
	if(BME280_Init(&bme1, &bme1_driver) != BME280_OK){

		while(1){} //do something when error occured
	}

	/* sensor's options */
	bme1_config.oversampling_h = BME280_OVERSAMPLING_X16;
	bme1_config.oversampling_p = BME280_OVERSAMPLING_X16;
	bme1_config.oversampling_t = BME280_OVERSAMPLING_X16;
	bme1_config.filter = BME280_FILTER_2;
	bme1_config.spi3w_enable = 0;
	bme1_config.mode = BME280_SLEEPMODE;

	/* set all sensor's options */
	if(BME280_ConfigureAll(&bme1, &bme1_config) != BME280_OK){

		while(1){} //do something when error occured
	}
}

int8_t bme280_read_platform_spec(uint8_t reg_addr, uint8_t *rxbuff, uint8_t rxlen, void *driver){

	/* check parameters */
	if((NULL == rxbuff) || (NULL == driver)) return -1;

	/* prepare local pointers */
	BME280_Driver_t *drv = (BME280_Driver_t *)driver;
	struct spi_rtos *spi = (struct spi_rtos *)drv->env_spec_data;

	/* take spi mutex */
	if(xSemaphoreTake(spi->spi_mutex, pdMS_TO_TICKS(1000)) != pdTRUE) return -1;

	/* set chip select low */
	HAL_GPIO_WritePin(spi->NCS_gpio, spi->NCS_pin, GPIO_PIN_RESET);

	/* send register's address to be read */
	if(HAL_SPI_Transmit(spi->spi_handle, &reg_addr, 1U, 200) != HAL_OK) {

		xSemaphoreGive(spi->spi_mutex);
		return -1;
	}

	/* read data */
	if(HAL_SPI_Receive(spi->spi_handle, rxbuff, rxlen, 200) != HAL_OK){

		xSemaphoreGive(spi->spi_mutex);
		return -1;
	}

	/* set chip select high */
	HAL_GPIO_WritePin(spi->NCS_gpio, spi->NCS_pin, GPIO_PIN_SET);

	xSemaphoreGive(spi->spi_mutex);
	return 0;
}

int8_t bme280_write_platform_spec(uint8_t reg_addr, uint8_t value, void *driver){

	/* check parameter */
	if(NULL == driver) return -1;

	/* prepare local pointers */
	BME280_Driver_t *drv = (BME280_Driver_t *)driver;
	struct spi_rtos *spi = (struct spi_rtos *)drv->env_spec_data;

	/* local buffer to store data to send */
	uint8_t buff[2];

	buff[0] = reg_addr & 0x7F;	// first element keeps address to be written (MSB must be reset in write mode!)
	buff[1] = value;			// second element keeps value to bw written

	/* take spi mutex */
	if(xSemaphoreTake(spi->spi_mutex, pdMS_TO_TICKS(1000)) != pdTRUE) return -1;

	/* set chip select low */
	HAL_GPIO_WritePin(spi->NCS_gpio, spi->NCS_pin, GPIO_PIN_RESET);

	/* send register's address and value at once */
	if(HAL_SPI_Transmit(spi->spi_handle, buff, 2U, 200) != HAL_OK){

		/* set chip select high */
		HAL_GPIO_WritePin(spi->NCS_gpio, spi->NCS_pin, GPIO_PIN_SET);
		xSemaphoreGive(spi->spi_mutex);
		return -1;
	}

	/* set chip select high */
	HAL_GPIO_WritePin(spi->NCS_gpio, spi->NCS_pin, GPIO_PIN_SET);

	xSemaphoreGive(spi->spi_mutex);
	return 0;
}

void bme280_delay_platform_spec(uint8_t delay_time){

	vTaskDelay(pdMS_TO_TICKS(delay_time));
}
/* USER CODE END Application */

