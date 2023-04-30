/**
 *******************************************
 * @file    bme280_definitions.h
 * @author  Łukasz Juraszek / JuraszekL
 * @version 1.0.0
 * @date	20.04.2023
 * @brief   Defined types for BME280 Driver
 * @note 	https://github.com/JuraszekL/BME280_Driver
 *******************************************
 *
 * @note https://www.bosch-sensortec.com/products/environmental-sensors/humidity-sensors-bme280/
 * @note https://www.bosch-sensortec.com/media/boschsensortec/downloads/datasheets/bst-bme280-ds002.pdf
*/

/**
 * @addtogroup BME280_Driver
 * @brief BME280 Driver
 * @{
 */

//***************************************

#ifndef BME280_DEFINITIONS_H
#define BME280_DEFINITIONS_H

//***************************************

#include <stdint.h>

/**
 * @defgroup BME280_DevStat BME280 Device Status
 * @brief Possible values of #initialized field that informs is the device properly initialized
 * @{
 */
#define BME280_NOT_INITIALIZED	(0x00) 	///< device is not initialized
#define BME280_INITIALIZED		(0x01)	///< device is initialized
///@}

/**
 * @defgroup BME280_Ret BME280 Returnd Values
 * @brief Values that can be returned by the driver's functions
 * @{
 */
#define BME280_OK				(0)		///< operation completed successfully
#define BME280_PARAM_ERR		(-1)	///< wrong parameters were passed to the function
#define BME280_INTERFACE_ERR	(-2)	///< user-defined function to communicate with sensor returned non-zero value
#define BME280_ID_ERR			(-3)	///< device ID doesn't match with #BME280_ID
#define BME280_NO_INIT_ERR		(-4)	///< device wasn't initialized properly and operation cannot be performed
#define BME280_CONDITION_ERR	(-5)	///< device is set to wrong operation mode, cannot perform operation
///@}

/**
 * @defgroup BME280_I2CAddr BME280 I2C Address
 * @brief Address on I2C bus depends of SDO pin connection. Use one of these values as #I2cAddr parameter in #BME280_Init function
 * @{
 */
#define BME280_I2CADDR_SDOL	(0x76)		///< if SDO pin is connected to GND
#define BME280_I2CADDR_SDOH	(0x77)		///< if SDO pin is connected to VCC
///@}

/**
 * @defgroup BME280_Regs BME280 Registers
 * @brief Definition of register addresses and lenghts
 * @{
 */
#define BME280_ID			(0x60)		///< Chip ID
#define BME280_ID_ADDR		(0xD0)		///< address of Chip ID

	/* Reset related */
#define BME280_RESET_ADDR	(0xE0)		///< address of Reset register
#define BME280_RESET_VALUE	(0xB6)		///< value that should be written to Reset register

	/* calibration data related */
#define BME280_CALIB_DATA1_ADDR	(0x88)	///< address of first block with calibration data
#define BME280_CALIB_DATA1_LEN	(25U)	///< lenght of first block with calibration data
#define BME280_CALIB_DATA2_ADDR	(0xE1)	///< address of second block with calibration data
#define BME280_CALIB_DATA2_LEN	(7U)	///< lenght of second block with calibration data

	/* control and config related */
#define BME280_CTRL_HUM_ADDR	(0xF2)	///< address of ctrl_hum register
#define BME280_CTRL_MEAS_ADDR	(0xF4)	///< address of ctrl_meas register
#define BME280_CONFIG_ADDR		(0xF5)	///< address of config register

	/* raw adc data related */
#define BME280_PRESS_ADC_ADDR	(0xF7)	///< address of pressure adc data
#define BME280_PRESS_ADC_LEN	(3U)	///< lenght of pressure adc data
#define BME280_TEMP_ADC_ADDR	(0xFA)	///< address of temperature adc data
#define BME280_TEMP_ADC_LEN		(3U)	///< lenght of temperature adc data
#define BME280_HUM_ADC_ADDR		(0xFD)	///< address of humidity adc data
#define BME280_HUM_ADC_LEN		(2U)	///< lenght of humidity adc data
///@}

/**
 * @defgroup BME280_Sett BME280 Settings
 * @brief Inernal sensor's settings that can be changed
 * @{
 */

/**
 *
 * @defgroup BME280_Ovs	Oversampling
 * @brief Possible values of oversampling.
 * @{
 */
	/* oversampling values */
#define BME280_OVERSAMPLING_SKIPP	(0x00)	///< no oversampling (skipp measure)
#define BME280_OVERSAMPLING_X1		(0x01)	///< oversampling x1
#define BME280_OVERSAMPLING_X2		(0x02)	///< oversampling x2
#define BME280_OVERSAMPLING_X4		(0x03)	///< oversampling x4
#define BME280_OVERSAMPLING_X8		(0x04)	///< oversampling x8
#define BME280_OVERSAMPLING_X16		(0x05)	///< oversampling x16
///@}

/**
 *
 * @defgroup BME280_mode Operating Mode
 * @brief Possible operating modes.
 * @{
 */
	/* operating mode */
#define BME280_SLEEPMODE	(0x00)	///< sleep mode
#define BME280_FORCEDMODE	(0x01)	///< forced mode
#define BME280_NORMALMODE	(0x03)	///< normal mode
///@}

/**
 *
 * @defgroup BME280_tstby Standby Time
 * @brief Possible values of standby time in normal mode
 * @{
 */
	/* standby time in normal mode  */
#define BME280_STBY_0_5MS	(0x00)	///< 0,5ms
#define BME280_STBY_62_5MS	(0x01)	///< 62,5ms
#define BME280_STBY_125MS	(0x02)	///< 125ms
#define BME280_STBY_250MS	(0x03)	///< 250ms
#define BME280_STBY_500MS	(0x04)	///< 500ms
#define BME280_STBY_1000MS	(0x05)	///< 1000ms
#define BME280_STBY_10MS	(0x06)	///< 10ms
#define BME280_STBY_20MS	(0x07)	///< 20ms
///@}

/**
 *
 * @defgroup BME280_filter IIR Filter
 * @brief Possible values of IIR filter settings
 * @{
 */
	/* filter coefficient */
#define BME280_FILTER_OFF	(0x00)	///< no IIR filter
#define BME280_FILTER_2		(0x01)	///< IIR filter coefficient 2
#define BME280_FILTER_4		(0x02)	///< IIR filter coefficient 4
#define BME280_FILTER_8		(0x03)	///< IIR filter coefficient 8
#define BME280_FILTER_16	(0x04)	///< IIR filter coefficient 16
///@}

///@}

/**
 * @defgroup BME280_prots Function pointers
 * @brief Platform speicific functions
 *
 * These functions have to be created by user and passed as agruments to #BME280_Init function.
 * @{
 */

/**
 * Function to read the data from sensor in burst mode.
 * @param[in] reg_addr address of register to be read (f.e. #BME280_ID_ADDR)
 * @param[in] *rxbuff pointer to the buffer where data will be stored
 * @param[in] rxlen lenght of data to be read (in bytes)
 * @param[in] dev_addr address of the device on I2C bus (#BME280_I2CADDR_SDOL or #BME280_I2CADDR_SDOH)
 * @param[in] env_spec_data pointer to platform specific data required to perform bus operation
 * (f.e. pointer to i2c bus strucure)
 * @return 0 success
 * @return -1 failure
 */
typedef int8_t (*bme280_readbytes)(uint8_t reg_addr, uint8_t *rxbuff, uint8_t rxlen, uint8_t dev_addr, void *env_spec_data);

/**
 * Function to write one byte to sensor.
 * @param[in] reg_addr address of register to be written (f.e. #BME280_RESET_ADDR)
 * @param[in] value value to write (f.e. #BME280_RESET_VALUE)
 * @param[in] dev_addr address of the device on I2C bus (#BME280_I2CADDR_SDOL or #BME280_I2CADDR_SDOH)
 * @param[in] env_spec_data pointer to platform specific data required to perform bus operation
 * (f.e. pointer to i2c bus strucure)
 * @return 0 success
 * @return -1 failure
 */
typedef int8_t (*bme280_writebyte)(uint8_t reg_addr, uint8_t value, uint8_t dev_addr, void *env_spec_data);

/**
 * Delay function.
 * @param[in] delay_time time to delay in miliseconds
 */
typedef void (*bme280_delayms)(uint8_t delay_time);
///@}

/**
 * @enum BME280_Mode_t
 * @brief BME280 operating mode
 *
 * Actual mode is always stored in #BME280_t structure and used to check correct conditions of actual operation.
 * @{
 */
typedef enum {

	sleep_mode = 0x00,	///< sensor is in sleep mide
	forced_mode = 0x01,	///< sensor is in forced mode
	normal_mode = 0x03	///< sensor is in normal mode

} BME280_Mode_t;
///@}

/**
 * @defgroup BME280_privtypedef Private typedefs
 * @brief Typedefs for internal calculations
 *
 * Change if needed
 * @{
 */
typedef int32_t BME280_S32_t;	///< signed 32-bit integer variable
typedef uint32_t BME280_U32_t;	///< unsigned 32-bit integer variable
typedef int64_t BME280_S64_t;	///< signed 64-bit integer variable
///@}

	/* structure keeps calibration data that were read from sensor */
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

	/* main structure where single sensor is cofigured */
typedef struct BME280_device {

	uint8_t i2c_address;
	void *env_spec_data;

	bme280_readbytes read;
	bme280_writebyte write;
	bme280_delayms delay;

	struct BME280_calibration_data trimm;
	BME280_S32_t t_fine;

	uint8_t initialized;
	BME280_Mode_t mode;

} BME280_t;

	/* structure used to configure every parameters in sensor at once */
typedef struct BME280_conf {

	uint8_t oversampling_h;
	uint8_t oversampling_p;
	uint8_t oversampling_t;
	uint8_t mode;
	uint8_t t_stby;
	uint8_t filter;
	uint8_t spi3w_enable;

} BME280_Config_t;

	/* structure used to present data from sensor (without floating point types) */
typedef struct BME280_data {

	/* 21.37 deg */
	int8_t temp_int;
	uint8_t temp_fract;

	/* 1001.891 hPa */
	uint16_t pressure_int;
	uint16_t pressure_fract;

	/* 49.274 % */
	uint8_t humidity_int;
	uint16_t humidity_fract;

} BME280_Data_t;

/* structure used to present data from sensor (with floating point types) */
typedef struct BME280_data_float {

	float temp;
	float press;
	float hum;

} BME280_DataF_t;

#endif /* BME280_DEFINITIONS_H */

///@}
