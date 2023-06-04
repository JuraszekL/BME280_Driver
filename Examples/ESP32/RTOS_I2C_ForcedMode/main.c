/**
  ******************************************************************************
  * File Name          : main.c
  * Description        : Code for freertos applications
  ******************************************************************************
  *
  * This is an example of use BME280_driver with ESP32 platform and FreeRTOS enviroment
  * Functions of this driver are not reentrant
  * so user is obligated to use mutexes to protect sensor's structure
  * and communication peripherials. Here we use only one sensor in one task
  * so mutex for BME280_t structure was skipped.
  * There is one sensor connected to I2C0, pin SDO is connected to GND
  * Results are stored as integers inside bme1_data structure.
  * Sensor if initialized, then configured. Inside app_main task single
  * measure is forced every 1000ms.
  *
  ******************************************************************************
  */

#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"
#include "driver/i2c.h"
#include "esp_log.h"

#include "bme280.h"

/* public typedefs */
struct i2c_rtos {

	i2c_port_t i2c_port_nr;
	SemaphoreHandle_t i2c_mutex;
};

/* public variables */
BME280_t bme1;
BME280_Driver_t bme1_driver;
BME280_Config_t bme1_config;
BME280_Data_t bme1_data;
struct i2c_rtos bme1_i2c;

/* function prototypes */
void i2c0_init(void);
void bme1_init(void);
int8_t bme280_read_platform_spec(uint8_t reg_addr, uint8_t *rxbuff, uint8_t rxlen, void *driver);
int8_t bme280_write_platform_spec(uint8_t reg_addr, uint8_t value, void *driver);
void bme280_delay_platform_spec(uint8_t delay_time);


	/* main */
void app_main(void){

	char tag[] = "app_main";
	int8_t res;

	i2c0_init();
	bme1_init();

	for(;;){

		/* print results from sensor once per second */
		vTaskDelay(pdMS_TO_TICKS(1000));

		res = BME280_ReadAllForce(&bme1, &bme1_data);
		if(res == BME280_OK){

			ESP_LOGI(tag, "BME280 nr.1:\nTemperature: %d,%d degC\nPressure: %d,%d hPa\nHumidity: %d,%d%%",
					bme1_data.temp_int, bme1_data.temp_fract, bme1_data.pressure_int, bme1_data.pressure_fract,
					bme1_data.humidity_int, bme1_data.humidity_fract);
		}
		else{

			ESP_LOGE(tag, "BME280 nr.1: error!, res = %d", res);
		}
    }
}


/* functions definitions */
void i2c0_init(void){

	char tag[] = "i2c0";
	i2c_config_t i2c0_conf;

	/* set I2C bus pins and parameters */
	i2c0_conf.mode = I2C_MODE_MASTER;
	i2c0_conf.scl_io_num = GPIO_NUM_13;
	i2c0_conf.sda_io_num = GPIO_NUM_14;
	i2c0_conf.scl_pullup_en = GPIO_PULLUP_DISABLE;
	i2c0_conf.sda_pullup_en = GPIO_PULLUP_DISABLE;
	i2c0_conf.master.clk_speed = 100000U;
	i2c0_conf.clk_flags = 0;

	if(i2c_param_config(I2C_NUM_0, &i2c0_conf) != ESP_OK){

		ESP_LOGE(tag, "param_config error");
		while(1){

			// do something if error occured
			vTaskDelay(pdMS_TO_TICKS(1000));
		}
	}

	if(i2c_driver_install(I2C_NUM_0, i2c0_conf.mode, 0, 0, 0) != ESP_OK){

		ESP_LOGE(tag, "driver_install error");
		while(1){

			// do something if error occured
			vTaskDelay(pdMS_TO_TICKS(1000));
		}
	}
}

void bme1_init(void){

	char tag[] = "bme1";

	/* fill platform specific driver data */
	bme1_i2c.i2c_port_nr = I2C_NUM_0;
	bme1_i2c.i2c_mutex = xSemaphoreCreateMutex();

	/* setup bme1 driver */
	bme1_driver.read = bme280_read_platform_spec;
	bme1_driver.write = bme280_write_platform_spec;
	bme1_driver.delay = bme280_delay_platform_spec;
	bme1_driver.env_spec_data = &bme1_i2c;
	bme1_driver.i2c_address = BME280_I2CADDR_SDOL;

	/* initialize sensor */
	if(BME280_Init(&bme1, &bme1_driver) != BME280_OK){

		ESP_LOGE(tag, "Init error");
		while(1){

			// do something if error occured
			vTaskDelay(pdMS_TO_TICKS(1000));
		}
	}

	/* sensor's options */
	bme1_config.oversampling_h = BME280_OVERSAMPLING_X4;
	bme1_config.oversampling_p = BME280_OVERSAMPLING_X2;
	bme1_config.oversampling_t = BME280_OVERSAMPLING_X2;
	bme1_config.filter = BME280_FILTER_2;
	bme1_config.spi3w_enable = 0;
	bme1_config.mode = BME280_SLEEPMODE;

	/* set all sensor's options */
	if(BME280_ConfigureAll(&bme1, &bme1_config) != BME280_OK){

		ESP_LOGE(tag, "ConfigureAll error");
		while(1){

			// do something if error occured
			vTaskDelay(pdMS_TO_TICKS(1000));
		}
	}
}

int8_t bme280_read_platform_spec(uint8_t reg_addr, uint8_t *rxbuff, uint8_t rxlen, void *driver){

	/* check parameter */
	if((NULL == rxbuff) || (NULL == driver)) return -1;

	/* prepare local variables */
	esp_err_t res;
	i2c_cmd_handle_t read_cmd;
	BME280_Driver_t *drv = (BME280_Driver_t *)driver;
	struct i2c_rtos *i2c = (struct i2c_rtos *)drv->env_spec_data;

	/* take i2c mutex */
	if(xSemaphoreTake(i2c->i2c_mutex, pdMS_TO_TICKS(1000)) != pdTRUE) return -1;

	/* create new command list */
	read_cmd = i2c_cmd_link_create();

	/* fill the list with proper commands */
	i2c_master_start(read_cmd);
	i2c_master_write_byte(read_cmd, (drv->i2c_address << 1U), 1);
	i2c_master_write_byte(read_cmd, reg_addr, 1);

	i2c_master_start(read_cmd);
	i2c_master_write_byte(read_cmd, ((drv->i2c_address << 1U) + 1), 1);
	i2c_master_read(read_cmd, rxbuff, rxlen, I2C_MASTER_LAST_NACK);
	i2c_master_stop(read_cmd);

	/* start i2c transmission */
	res = i2c_master_cmd_begin(i2c->i2c_port_nr, read_cmd, pdMS_TO_TICKS(500));

	/* delete command list */
	i2c_cmd_link_delete(read_cmd);

	/* give the mutex back */
	xSemaphoreGive(i2c->i2c_mutex);

	if(ESP_OK != res) return -1;
	return 0;
}

int8_t bme280_write_platform_spec(uint8_t reg_addr, uint8_t value, void *driver){

	/* check parameter */
	if(NULL == driver) return -1;

	/* prepare local variables */
	esp_err_t res;
	i2c_cmd_handle_t write_cmd;
	BME280_Driver_t *drv = (BME280_Driver_t *)driver;
	struct i2c_rtos *i2c = (struct i2c_rtos *)drv->env_spec_data;

	/* take i2c mutex */
	if(xSemaphoreTake(i2c->i2c_mutex, pdMS_TO_TICKS(1000)) != pdTRUE) return -1;

	/* create new command list */
	write_cmd = i2c_cmd_link_create();

	/* fill the list with proper commands */
	i2c_master_start(write_cmd);
	i2c_master_write_byte(write_cmd, (drv->i2c_address << 1U), 1);
	i2c_master_write_byte(write_cmd, reg_addr, 1);
	i2c_master_write_byte(write_cmd, value, 1);
	i2c_master_stop(write_cmd);

	/* start i2c transmission */
	res = i2c_master_cmd_begin(i2c->i2c_port_nr, write_cmd, pdMS_TO_TICKS(500));

	/* delete command list */
	i2c_cmd_link_delete(write_cmd);

	/* give the mutex back */
	xSemaphoreGive(i2c->i2c_mutex);

	if(ESP_OK != res) return -1;
	return 0;
}

void bme280_delay_platform_spec(uint8_t delay_time){

	/* never call vTaskDelay with 0 ticks delay */
	uint8_t ticks = pdMS_TO_TICKS(delay_time);
	if(0 == ticks) ticks = 1;
	vTaskDelay(ticks);
}
