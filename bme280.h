/**
 *******************************************
 * @file    bme280.h
 * @author  Łukasz Juraszek / JuraszekL
 * @version 1.0.0
 * @date	20.04.2023
 * @brief   Header for BME280 Driver
 * @note 	https://github.com/JuraszekL/BME280_Driver
 *******************************************
 *
 * @note https://www.bosch-sensortec.com/products/environmental-sensors/humidity-sensors-bme280/
 * @note https://www.bosch-sensortec.com/media/boschsensortec/downloads/datasheets/bst-bme280-ds002.pdf
*/

/**
 * @addtogroup BME280_Driver
 * @{
 */

//***************************************

#ifndef BME280_H
#define BME280_H

//***************************************

#ifdef __cplusplus /* CPP */
extern "C" {
#endif

//***************************************

#include "bme280_definitions.h"

/**
 * @defgroup BME280_libconf Library Configuration
 * @brief Set library options here
 * @{
 */
/// comment this line if you don't want to use 64bit variables in calculations
#define USE_64BIT
/// comment this line if u don't need to use floating point results
#define USE_FLOAT
///@}

/**
 * @defgroup BME280_Pubfunc Public functions
 * @brief Use these functions only
 * @{
 */

/**
 * @brief Function to initialize sensor and resources
 * @note This must be a first function usend in code before any other opearion can be performed!
 *
 * Init funtion performs sensor reset and checks #BME280_ID. It doesn't set any sensor's parameters. Calibration
 * data specific for each one sensor are read while Init function. If operation is completed with
 * success function sets #BME280_INITIALIZED value in #BME280_t structure.
 * @param[in] *Dev pointer to #BME280_t structure which should be initialized
 * @param[in] I2cAddr value of sensor's I2C address, should be #BME280_I2CADDR_SDOL or #BME280_I2CADDR_SDOL only
 * @param[in] *EnvSpecData pointer to platform specific data which are required to transfer data (f.e. pointer
 * to i2c structure)
 * @param[in] ReadFun pointer to user-created function that will be used to read data from sensor
 * @param[in] WriteFun pointer to user-created function that will be used to write data to sensor
 * @param[in] Delay pointer to user-created delay function
 * @return #BME280_OK success
 * @return #BME280_PARAM_ERR wrong parameter passed
 * @return #BME280_INTERFACE_ERR user defined read/write function returned non-zero value
 * @return #BME280_ID_ERR sensor's id doesnt match with #BME280_ID
 */
int8_t BME280_Init(BME280_t *Dev, uint8_t I2cAddr, void *EnvSpecData,
		bme280_readbytes ReadFun, bme280_writebyte WriteFun, bme280_delayms Delay);

/**
 * @brief Function to set all sensor settings at once
 * @note Sensor must be in #BME280_SLEEPMODE to set all parameters. Force #BME280_SLEEPMODE with #BME280_SetMode
 * function before or use it directly after #BME280_Init function only.
 *
 * Function writes all 3 config registers without reading them before. It can be usefull after power-up or reset.
 * @param[in] *Dev pointer to sensor's #BME280_t structure
 * @param[in] *Config pointer to #BME280_Config_t structure which contains all paramaters to be set
 * @return #BME280_OK success
 * @return #BME280_PARAM_ERR wrong parameter passed
 * @return #BME280_INTERFACE_ERR user defined read/write function returned non-zero value
 * @return #BME280_NO_INIT_ERR sensor was not initialized before
 * @return #BME280_CONDITION_ERR sensor is not in #BME280_SLEEPMODE
 */
int8_t BME280_ConfigureAll(BME280_t *Dev, BME280_Config_t *Config);

/**
 * @brief Function to perform sensor's software reset
 *
 * Function sends #BME280_RESET_VALUE to #BME280_RESET_ADDR. To perform this operation #bme280_writebyte
 * function must be set inside #BME280_t *Dev structure. Function sets #sleep_mode inside *Dev structure after reset.
 * @param[in] *Dev pointer to sensor's #BME280_t structure
 * @return #BME280_OK success
 * @return #BME280_PARAM_ERR wrong parameter passed or #bme280_writebyte function is not set
 * @return #BME280_INTERFACE_ERR user defined read/write function returned non-zero value
 */
int8_t BME280_Reset(BME280_t *Dev);

int8_t BME280_GetMode(BME280_t *Dev, uint8_t *Mode);
int8_t BME280_SetMode(BME280_t *Dev, uint8_t Mode);

int8_t BME280_GetPOvs(BME280_t *Dev, uint8_t *POvs);
int8_t BME280_SetPOvs(BME280_t *Dev, uint8_t POvs);

int8_t BME280_GetTOvs(BME280_t *Dev, uint8_t *TOvs);
int8_t BME280_SetTOvs(BME280_t *Dev, uint8_t TOvs);

int8_t BME280_GetHOvs(BME280_t *Dev, uint8_t *HOvs);
int8_t BME280_SetHOvs(BME280_t *Dev, uint8_t HOvs);

int8_t BME280_GetTStby(BME280_t *Dev, uint8_t *TStby);
int8_t BME280_SetTStby(BME280_t *Dev, uint8_t TStby);

int8_t BME280_GetTFilter(BME280_t *Dev, uint8_t *Filter);
int8_t BME280_SetFilter(BME280_t *Dev, uint8_t Filter);

int8_t BME280_Enable3WireSPI(BME280_t *Dev);
int8_t BME280_Disable3WireSPI(BME280_t *Dev);
int8_t BME280_Is3WireSPIEnabled(BME280_t *Dev, uint8_t *Result);

int8_t BME280_ReadLastAll(BME280_t *Dev, BME280_Data_t *Data);
int8_t BME280_ReadLastAll_F(BME280_t *Dev, BME280_DataF_t *Data);

int8_t BME280_ReadLastTemp(BME280_t *Dev, int8_t *TempInt, uint8_t *TempFract);
int8_t BME280_ReadLastTemp_F(BME280_t *Dev, float *Temp);

int8_t BME280_ReadLastPress(BME280_t *Dev, uint16_t *PressInt, uint16_t *PressFract);
int8_t BME280_ReadLastPress_F(BME280_t *Dev, float *Press);

int8_t BME280_ReadLastHum(BME280_t *Dev, uint8_t *HumInt, uint16_t *HumFract);
int8_t BME280_ReadLastHum_F(BME280_t *Dev, float *Hum);
///@}


//***************************************

#ifdef __cplusplus
}
#endif /* CPP */

//***************************************

#endif /* BME280_H */

///@}
