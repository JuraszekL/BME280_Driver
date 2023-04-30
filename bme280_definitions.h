/*
 * This is a non-commercial project for learning purposes only. Feel free to use it.
 *
 * JuraszekL
 *
 * */

//***************************************

#ifndef BME280_DEFINITIONS_H
#define BME280_DEFINITIONS_H

//***************************************

#include <stdint.h>

//***************************************
/* general */
//***************************************

	/* possible states of sensor in BME280_device.initialized */
#define BME280_NOT_INITIALIZED	(0x00)
#define BME280_INITIALIZED		(0x01)

//***************************************
/* returned values */
//***************************************

#define BME280_OK				(0)
#define BME280_PARAM_ERR		(-1)
#define BME280_INTERFACE_ERR	(-2)
#define BME280_ID_ERR			(-3)
#define BME280_NO_INIT_ERR		(-4)
#define BME280_CONDITION_ERR	(-5)

//***************************************
/* I2C address */
//***************************************

// if SDO pin is connected to GND
#define BME280_I2CADDR_SDOL	(0x76)
// if SDO pin is connected to VCC
#define BME280_I2CADDR_SDOH	(0x77)

//***************************************
/* registers values */
//***************************************

	/* ID related */
#define BME280_ID			(0x60)
#define BME280_ID_ADDR		(0xD0)

	/* Reset related */
#define BME280_RESET_ADDR	(0xE0)
#define BME280_RESET_VALUE	(0xB6)

	/* calibration data related */
#define BME280_CALIB_DATA1_ADDR	(0x88)
#define BME280_CALIB_DATA1_LEN	(25U)
#define BME280_CALIB_DATA2_ADDR	(0xE1)
#define BME280_CALIB_DATA2_LEN	(7U)

	/* control and config related */
#define BME280_CTRL_HUM_ADDR	(0xF2)
#define BME280_CTRL_MEAS_ADDR	(0xF4)
#define BME280_CONFIG_ADDR		(0xF5)

	/* raw adc data related */
#define BME280_PRESS_ADC_ADDR	(0xF7)
#define BME280_PRESS_ADC_LEN	(3U)
#define BME280_TEMP_ADC_ADDR	(0xFA)
#define BME280_TEMP_ADC_LEN		(3U)
#define BME280_HUM_ADC_ADDR		(0xFD)
#define BME280_HUM_ADC_LEN		(2U)

//***************************************
/* settings */
//***************************************

	/* oversampling values */
#define BME280_OVERSAMPLING_SKIPP	(0x00)
#define BME280_OVERSAMPLING_X1		(0x01)
#define BME280_OVERSAMPLING_X2		(0x02)
#define BME280_OVERSAMPLING_X4		(0x03)
#define BME280_OVERSAMPLING_X8		(0x04)
#define BME280_OVERSAMPLING_X16		(0x05)

	/* operating mode */
#define BME280_SLEEPMODE	(0x00)
#define BME280_FORCEDMODE	(0x01)
#define BME280_NORMALMODE	(0x03)

	/* standby time in normal mode  */
#define BME280_STBY_0_5MS	(0x00)
#define BME280_STBY_62_5MS	(0x01)
#define BME280_STBY_125MS	(0x02)
#define BME280_STBY_250MS	(0x03)
#define BME280_STBY_500MS	(0x04)
#define BME280_STBY_1000MS	(0x05)
#define BME280_STBY_10MS	(0x06)
#define BME280_STBY_20MS	(0x07)

	/* filter coefficient */
#define BME280_FILTER_OFF	(0x00)
#define BME280_FILTER_2		(0x01)
#define BME280_FILTER_4		(0x02)
#define BME280_FILTER_8		(0x03)
#define BME280_FILTER_16	(0x04)

//***************************************
/* typedefs */
//***************************************

	/* pointers prototypes to user-defined and platform-dependent functions to
	 * comminucate with sensor */
typedef int8_t (*bme280_readbytes)(uint8_t reg_addr, uint8_t *rxbuff, uint8_t rxlen, uint8_t dev_addr, void *env_spec_data);
typedef int8_t (*bme280_writebyte)(uint8_t reg_addr, uint8_t value, uint8_t dev_addr, void *env_spec_data);
typedef void (*bme280_delayms)(uint8_t delay_time);

typedef enum {sleep_mode = 0x00, forced_mode = 0x01, normal_mode = 0x03} BME280_Mode_t;

	/* types used for compensation formules */
typedef int32_t BME280_S32_t;
typedef uint32_t BME280_U32_t;
typedef int64_t BME280_S64_t;

	/* main structure where single sensor is cofigured */
typedef struct BME280_device BME280_t;

	/* structure used to configure every parameters in sensor at once */
typedef struct BME280_conf BME280_Config_t;

	/* structure used to present data from sensor (without floating point types) */
typedef struct BME280_data BME280_Data_t;

/* structure used to present data from sensor (with floating point types) */
typedef struct BME280_data_float BME280_DataF_t;

struct BME280_calibration_data {

	uint16_t dig_T1;
	int16_t dig_T2;
	int16_t dig_T3;

	uint16_t dig_P1;
	int16_t dig_P2;
	int16_t dig_P3;
	int16_t dig_P4;
	int16_t dig_P5;
	int16_t dig_P6;
	int16_t dig_P7;
	int16_t dig_P8;
	int16_t dig_P9;

	uint8_t dig_H1;
	int16_t dig_H2;
	uint8_t dig_H3;
	int16_t dig_H4;
	int16_t dig_H5;
	int8_t dig_H6;
};

struct BME280_device {

	uint8_t i2c_address;
	void *env_spec_data;

	bme280_readbytes read;
	bme280_writebyte write;
	bme280_delayms delay;

	struct BME280_calibration_data trimm;
	BME280_S32_t t_fine;

	uint8_t initialized;
	BME280_Mode_t mode;
};

struct BME280_conf {

	uint8_t oversampling_h;
	uint8_t oversampling_p;
	uint8_t oversampling_t;
	uint8_t mode;
	uint8_t t_stby;
	uint8_t filter;
	uint8_t spi3w_enable;
};

struct BME280_data {

	/* 21.37 deg */
	int8_t temp_int;
	uint8_t temp_fract;

	/* 1001.891 hPa */
	uint16_t pressure_int;
	uint16_t pressure_fract;

	/* 49.274 % */
	uint8_t humidity_int;
	uint16_t humidity_fract;
};

struct BME280_data_float {

	float temp;
	float press;
	float hum;
};

#endif /* BME280_DEFINITIONS_H */
