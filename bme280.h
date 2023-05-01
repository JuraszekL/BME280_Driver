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

/**
 * @defgroup BME280_getfunctions Get Functions
 * @brief read sensor's settings
 * @{
 */

/**
 * @brief Function gets current @ref BME280_mode from sensor
 * @param[in] *Dev pointer to sensor's #BME280_t structure
 * @param[out] *Mode pointer to vartiable where result will be stored
 * @return #BME280_OK success
 * @return #BME280_PARAM_ERR wrong parameter passed
 * @return #BME280_NO_INIT_ERR sensor was not initialized before
 * @return #BME280_INTERFACE_ERR user defined read/write function returned non-zero value
 */
int8_t BME280_GetMode(BME280_t *Dev, uint8_t *Mode);

/**
 * @brief Function gets current pressure @ref BME280_Ovs from sensor
 * @param[in] *Dev pointer to sensor's #BME280_t structure
 * @param[out] *POvs pointer to vartiable where result will be stored
 * @return #BME280_OK success
 * @return #BME280_PARAM_ERR wrong parameter passed
 * @return #BME280_NO_INIT_ERR sensor was not initialized before
 * @return #BME280_INTERFACE_ERR user defined read/write function returned non-zero value
 */
int8_t BME280_GetPOvs(BME280_t *Dev, uint8_t *POvs);

/**
 * @brief Function gets current temperature @ref BME280_Ovs from sensor
 * @param[in] *Dev pointer to sensor's #BME280_t structure
 * @param[out] *TOvs pointer to vartiable where result will be stored
 * @return #BME280_OK success
 * @return #BME280_PARAM_ERR wrong parameter passed
 * @return #BME280_NO_INIT_ERR sensor was not initialized before
 * @return #BME280_INTERFACE_ERR user defined read/write function returned non-zero value
 */
int8_t BME280_GetTOvs(BME280_t *Dev, uint8_t *TOvs);

/**
 * @brief Function gets current humidity @ref BME280_Ovs from sensor
 * @param[in] *Dev pointer to sensor's #BME280_t structure
 * @param[out] *HOvs pointer to vartiable where result will be stored
 * @return #BME280_OK success
 * @return #BME280_PARAM_ERR wrong parameter passed
 * @return #BME280_NO_INIT_ERR sensor was not initialized before
 * @return #BME280_INTERFACE_ERR user defined read/write function returned non-zero value
 */
int8_t BME280_GetHOvs(BME280_t *Dev, uint8_t *HOvs);

/**
 * @brief Function gets current @ref BME280_tstby from sensor
 * @param[in] *Dev pointer to sensor's #BME280_t structure
 * @param[out] *TStby pointer to vartiable where result will be stored
 * @return #BME280_OK success
 * @return #BME280_PARAM_ERR wrong parameter passed
 * @return #BME280_NO_INIT_ERR sensor was not initialized before
 * @return #BME280_INTERFACE_ERR user defined read/write function returned non-zero value
 */
int8_t BME280_GetTStby(BME280_t *Dev, uint8_t *TStby);

/**
 * @brief Function gets current @ref BME280_filter coeficient from sensor
 * @param[in] *Dev pointer to sensor's #BME280_t structure
 * @param[out] *Filter pointer to vartiable where result will be stored
 * @return #BME280_OK success
 * @return #BME280_PARAM_ERR wrong parameter passed
 * @return #BME280_NO_INIT_ERR sensor was not initialized before
 * @return #BME280_INTERFACE_ERR user defined read/write function returned non-zero value
 */
int8_t BME280_GetTFilter(BME280_t *Dev, uint8_t *Filter);

/**
 * @brief Function checks if 3-wire SPI is enabled
 * @param[in] *Dev pointer to sensor's #BME280_t structure
 * @param[out] *Result pointer to vartiable where result will be stored, 0 - disabled, 1 - enabled
 * @return #BME280_OK success
 * @return #BME280_PARAM_ERR wrong parameter passed
 * @return #BME280_NO_INIT_ERR sensor was not initialized before
 * @return #BME280_INTERFACE_ERR user defined read/write function returned non-zero value
 */
int8_t BME280_Is3WireSPIEnabled(BME280_t *Dev, uint8_t *Result);
///@}

/**
 * @defgroup BME280_setfunctions Set Functions
 * @brief change sensor's settings
 * @note Sensor must be in #BME280_SLEEPMODE to change settings. Only #BME280_SetMode function can be used when
 * sensors is in different working mode,
 * @{
 */

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
 * @brief Function sets sensor's @ref BME280_mode
 *
 * Function reads single register from sensor, and checks if current mode matches mode requested by user.
 * If matches, function skips write operation and returns #BME280_OK. If doesnt, it prepares and sends new register
 * value, then sets correct #BME280_Mode_t inside *Dev structure.
 * @param[in] *Dev pointer to sensor's #BME280_t structure
 * @param[in] Mode value to be set, must be in range of @ref BME280_mode
 * @return #BME280_OK success
 * @return #BME280_PARAM_ERR wrong parameter passed
 * @return #BME280_INTERFACE_ERR user defined read/write function returned non-zero value
 * @return #BME280_NO_INIT_ERR sensor was not initialized before
 */
int8_t BME280_SetMode(BME280_t *Dev, uint8_t Mode);

/**
 * @brief Function sets sensor's pressure @ref BME280_Ovs
 * @note Sensor must be in #BME280_SLEEPMODE to set this parameter. Force #BME280_SLEEPMODE with #BME280_SetMode
 * function before or use it directly after #BME280_Init function only.
 *
 * Function reads single register from sensor, and checks if current value matches value requested by user.
 * If matches, function skips write operation and returns #BME280_OK. If doesnt, it prepares and sends new register
 * value.
 * @param[in] *Dev pointer to sensor's #BME280_t structure
 * @param[in] POvs value to be set, must be in range of @ref BME280_Ovs
 * @return #BME280_OK success
 * @return #BME280_PARAM_ERR wrong parameter passed
 * @return #BME280_INTERFACE_ERR user defined read/write function returned non-zero value
 * @return #BME280_NO_INIT_ERR sensor was not initialized before
 * @return #BME280_CONDITION_ERR sensor is not in #BME280_SLEEPMODE
 */
int8_t BME280_SetPOvs(BME280_t *Dev, uint8_t POvs);

/**
 * @brief Function sets sensor's temperature @ref BME280_Ovs
 * @note Sensor must be in #BME280_SLEEPMODE to set this parameter. Force #BME280_SLEEPMODE with #BME280_SetMode
 * function before or use it directly after #BME280_Init function only.
 *
 * Function reads single register from sensor, and checks if current value matches value requested by user.
 * If matches, function skips write operation and returns #BME280_OK. If doesnt, it prepares and sends new register
 * value.
 * @param[in] *Dev pointer to sensor's #BME280_t structure
 * @param[in] TOvs value to be set, must be in range of @ref BME280_Ovs
 * @return #BME280_OK success
 * @return #BME280_PARAM_ERR wrong parameter passed
 * @return #BME280_INTERFACE_ERR user defined read/write function returned non-zero value
 * @return #BME280_NO_INIT_ERR sensor was not initialized before
 * @return #BME280_CONDITION_ERR sensor is not in #BME280_SLEEPMODE
 */
int8_t BME280_SetTOvs(BME280_t *Dev, uint8_t TOvs);

/**
 * @brief Function sets sensor's humidity @ref BME280_Ovs
 * @note Sensor must be in #BME280_SLEEPMODE to set this parameter. Force #BME280_SLEEPMODE with #BME280_SetMode
 * function before or use it directly after #BME280_Init function only.
 *
 * Function reads single register from sensor, and checks if current value matches value requested by user.
 * If matches, function skips write operation and returns #BME280_OK. If doesnt, it prepares and sends new register
 * value.
 * @param[in] *Dev pointer to sensor's #BME280_t structure
 * @param[in] HOvs value to be set, must be in range of @ref BME280_Ovs
 * @return #BME280_OK success
 * @return #BME280_PARAM_ERR wrong parameter passed
 * @return #BME280_INTERFACE_ERR user defined read/write function returned non-zero value
 * @return #BME280_NO_INIT_ERR sensor was not initialized before
 * @return #BME280_CONDITION_ERR sensor is not in #BME280_SLEEPMODE
 */
int8_t BME280_SetHOvs(BME280_t *Dev, uint8_t HOvs);

/**
 * @brief Function sets sensor's @ref BME280_tstby
 * @note Sensor must be in #BME280_SLEEPMODE to set this parameter. Force #BME280_SLEEPMODE with #BME280_SetMode
 * function before or use it directly after #BME280_Init function only.
 *
 * Function reads single register from sensor, and checks if current value matches value requested by user.
 * If matches, function skips write operation and returns #BME280_OK. If doesnt, it prepares and sends new register
 * value.
 * @param[in] *Dev pointer to sensor's #BME280_t structure
 * @param[in] TStby value to be set, must be in range of @ref BME280_tstby
 * @return #BME280_OK success
 * @return #BME280_PARAM_ERR wrong parameter passed
 * @return #BME280_INTERFACE_ERR user defined read/write function returned non-zero value
 * @return #BME280_NO_INIT_ERR sensor was not initialized before
 * @return #BME280_CONDITION_ERR sensor is not in #BME280_SLEEPMODE
 */
int8_t BME280_SetTStby(BME280_t *Dev, uint8_t TStby);

/**
 * @brief Function sets sensor's @ref BME280_filter coeficient
 * @note Sensor must be in #BME280_SLEEPMODE to set this parameter. Force #BME280_SLEEPMODE with #BME280_SetMode
 * function before or use it directly after #BME280_Init function only.
 *
 * Function reads single register from sensor, and checks if current value matches value requested by user.
 * If matches, function skips write operation and returns #BME280_OK. If doesnt, it prepares and sends new register
 * value.
 * @param[in] *Dev pointer to sensor's #BME280_t structure
 * @param[in] Filter value to be set, must be in range of @ref BME280_filter
 * @return #BME280_OK success
 * @return #BME280_PARAM_ERR wrong parameter passed
 * @return #BME280_INTERFACE_ERR user defined read/write function returned non-zero value
 * @return #BME280_NO_INIT_ERR sensor was not initialized before
 * @return #BME280_CONDITION_ERR sensor is not in #BME280_SLEEPMODE
 */
int8_t BME280_SetFilter(BME280_t *Dev, uint8_t Filter);

/**
 * @brief Function enables 3-wire SPI bus
 * @note Sensor must be in #BME280_SLEEPMODE to set this parameter. Force #BME280_SLEEPMODE with #BME280_SetMode
 * function before or use it directly after #BME280_Init function only.
 *
 * Function reads single register from sensor, and checks if current value matches value requested by user.
 * If matches, function skips write operation and returns #BME280_OK. If doesnt, it prepares and sends new register
 * value.
 * @param[in] *Dev pointer to sensor's #BME280_t structure
 * @return #BME280_OK success
 * @return #BME280_PARAM_ERR wrong parameter passed
 * @return #BME280_INTERFACE_ERR user defined read/write function returned non-zero value
 * @return #BME280_NO_INIT_ERR sensor was not initialized before
 * @return #BME280_CONDITION_ERR sensor is not in #BME280_SLEEPMODE
 */
int8_t BME280_Enable3WireSPI(BME280_t *Dev);

/**
 * @brief Function disables 3-wire SPI bus
 * @note Sensor must be in #BME280_SLEEPMODE to set this parameter. Force #BME280_SLEEPMODE with #BME280_SetMode
 * function before or use it directly after #BME280_Init function only.
 *
 * Function reads single register from sensor, and checks if current value matches value requested by user.
 * If matches, function skips write operation and returns #BME280_OK. If doesnt, it prepares and sends new register
 * value.
 * @param[in] *Dev pointer to sensor's #BME280_t structure
 * @return #BME280_OK success
 * @return #BME280_PARAM_ERR wrong parameter passed
 * @return #BME280_INTERFACE_ERR user defined read/write function returned non-zero value
 * @return #BME280_NO_INIT_ERR sensor was not initialized before
 * @return #BME280_CONDITION_ERR sensor is not in #BME280_SLEEPMODE
 */
int8_t BME280_Disable3WireSPI(BME280_t *Dev);
///@}

/**
 * @defgroup BME280_readnofl Read Functions (int)
 * @brief read measured data from sensor as integers
 * @{
 */

/**
 * @brief Function reads all measured data at once
 * @note Sensor must be in #BME280_NORMALMODE to read last measured values.
 *
 * Function reads all adc values from sensor, converts them into single variables and compensate
 * with use #BME280_calibration_data. Compensated values are then converted into #BME280_Data_t structure.
 * @param[in] *Dev pointer to sensor's #BME280_t structure
 * @param[out] *Data pointer to structure where result will be stored
 * @return #BME280_OK success
 * @return #BME280_PARAM_ERR wrong parameter passed
 * @return #BME280_INTERFACE_ERR user defined read/write function returned non-zero value
 * @return #BME280_NO_INIT_ERR sensor was not initialized before
 * @return #BME280_CONDITION_ERR sensor is not in #BME280_SLEEPMODE
 */
int8_t BME280_ReadLastAll(BME280_t *Dev, BME280_Data_t *Data);

/**
 * @brief Function reads last measured temperature
 * @note Sensor must be in #BME280_NORMALMODE to read last measured values.
 *
 * Function reads temperature related adc values from sensor, converts them into single variable and compensate
 * with use #BME280_calibration_data. Compensated value is then converted into integer part and fractial part and
 * stored in external variables
 * @param[in] *Dev pointer to sensor's #BME280_t structure
 * @param[out] *TempInt pointer to variable where integer part of temperature will be stored
 * @param[out] *TempFract pointer to variable where fractial part of temperature will be stored
 * @return #BME280_OK success
 * @return #BME280_PARAM_ERR wrong parameter passed
 * @return #BME280_INTERFACE_ERR user defined read/write function returned non-zero value
 * @return #BME280_NO_INIT_ERR sensor was not initialized before
 * @return #BME280_CONDITION_ERR sensor is not in #BME280_SLEEPMODE
 */
int8_t BME280_ReadLastTemp(BME280_t *Dev, int8_t *TempInt, uint8_t *TempFract);

/**
 * @brief Function reads last measured pressure
 * @note Sensor must be in #BME280_NORMALMODE to read last measured values.
 *
 * Function reads temperature and pressure related adc values from sensor, converts them into single variables and compensate
 * with use #BME280_calibration_data. Compensated value is then converted into integer part and fractial part and
 * stored in external variables
 * @param[in] *Dev pointer to sensor's #BME280_t structure
 * @param[out] *PressInt pointer to variable where integer part of pressure will be stored
 * @param[out] *PressFract pointer to variable where fractial part of pressure will be stored
 * @return #BME280_OK success
 * @return #BME280_PARAM_ERR wrong parameter passed
 * @return #BME280_INTERFACE_ERR user defined read/write function returned non-zero value
 * @return #BME280_NO_INIT_ERR sensor was not initialized before
 * @return #BME280_CONDITION_ERR sensor is not in #BME280_SLEEPMODE
 */
int8_t BME280_ReadLastPress(BME280_t *Dev, uint16_t *PressInt, uint16_t *PressFract);

/**
 * @brief Function reads last measured humidity
 * @note Sensor must be in #BME280_NORMALMODE to read last measured values.
 *
 * Function reads temperature and humidity related adc values from sensor, converts them into single variables and compensate
 * with use #BME280_calibration_data. Compensated value is then converted into integer part and fractial part and
 * stored in external variables
 * @param[in] *Dev pointer to sensor's #BME280_t structure
 * @param[out] *HumInt pointer to variable where integer part of humidity will be stored
 * @param[out] *HumFract pointer to variable where fractial part of humidity will be stored
 * @return #BME280_OK success
 * @return #BME280_PARAM_ERR wrong parameter passed
 * @return #BME280_INTERFACE_ERR user defined read/write function returned non-zero value
 * @return #BME280_NO_INIT_ERR sensor was not initialized before
 * @return #BME280_CONDITION_ERR sensor is not in #BME280_SLEEPMODE
 */
int8_t BME280_ReadLastHum(BME280_t *Dev, uint8_t *HumInt, uint16_t *HumFract);
///@}

#ifdef USE_FLOAT
/**
 * @defgroup BME280_readfl Read Functions (float)
 * @brief read measured data from sensor as floating point values
 * @note #USE_FLOAT in @ref BME280_libconf must be uncommented to use these functions
 * @{
 */

/**
 * @brief Function reads all measured data at once
 * @note Sensor must be in #BME280_NORMALMODE to read last measured values.
 *
 * Function reads all adc values from sensor, converts them into single variables and compensate
 * with use #BME280_calibration_data. Compensated values are then converted into #BME280_DataF_t structure.
 * @param[in] *Dev pointer to sensor's #BME280_t structure
 * @param[out] *Data pointer to structure where result will be stored
 * @return #BME280_OK success
 * @return #BME280_PARAM_ERR wrong parameter passed
 * @return #BME280_INTERFACE_ERR user defined read/write function returned non-zero value
 * @return #BME280_NO_INIT_ERR sensor was not initialized before
 * @return #BME280_CONDITION_ERR sensor is not in #BME280_SLEEPMODE
 */
int8_t BME280_ReadLastAll_F(BME280_t *Dev, BME280_DataF_t *Data);

/**
 * @brief Function reads last measured temperature
 * @note Sensor must be in #BME280_NORMALMODE to read last measured values.
 *
 * Function reads temperature related adc values from sensor, converts them into single variable and compensate
 * with use #BME280_calibration_data. Compensated value is then converted into floating point value and stored in
 * external variable
 * @param[in] *Dev pointer to sensor's #BME280_t structure
 * @param[out] *Temp pointer to variable where temperature will be stored
 * @return #BME280_OK success
 * @return #BME280_PARAM_ERR wrong parameter passed
 * @return #BME280_INTERFACE_ERR user defined read/write function returned non-zero value
 * @return #BME280_NO_INIT_ERR sensor was not initialized before
 * @return #BME280_CONDITION_ERR sensor is not in #BME280_SLEEPMODE
 */
int8_t BME280_ReadLastTemp_F(BME280_t *Dev, float *Temp);

/**
 * @brief Function reads last measured pressure
 * @note Sensor must be in #BME280_NORMALMODE to read last measured values.
 *
 * Function reads temperature and pressure related adc values from sensor, converts them into single variables and compensate
 * with use #BME280_calibration_data. Compensated value is then converted into floating point value and stored in
 * external variable
 * @param[in] *Dev pointer to sensor's #BME280_t structure
 * @param[out] *Press pointer to variable where temperature will be stored
 * @return #BME280_OK success
 * @return #BME280_PARAM_ERR wrong parameter passed
 * @return #BME280_INTERFACE_ERR user defined read/write function returned non-zero value
 * @return #BME280_NO_INIT_ERR sensor was not initialized before
 * @return #BME280_CONDITION_ERR sensor is not in #BME280_SLEEPMODE
 */
int8_t BME280_ReadLastPress_F(BME280_t *Dev, float *Press);

/**
 * @brief Function reads last measured humidity
 * @note Sensor must be in #BME280_NORMALMODE to read last measured values.
 *
 * Function reads temperature and humidity related adc values from sensor, converts them into single variables and compensate
 * with use #BME280_calibration_data. Compensated value is then converted into floating point value and stored in
 * external variable
 * @param[in] *Dev pointer to sensor's #BME280_t structure
 * @param[out] *Hum pointer to variable where temperature will be stored
 * @return #BME280_OK success
 * @return #BME280_PARAM_ERR wrong parameter passed
 * @return #BME280_INTERFACE_ERR user defined read/write function returned non-zero value
 * @return #BME280_NO_INIT_ERR sensor was not initialized before
 * @return #BME280_CONDITION_ERR sensor is not in #BME280_SLEEPMODE
 */
int8_t BME280_ReadLastHum_F(BME280_t *Dev, float *Hum);
#endif
///@}
///@}


//***************************************

#ifdef __cplusplus
}
#endif /* CPP */

//***************************************

#endif /* BME280_H */

///@}
