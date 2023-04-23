/*
 * This is a non-commercial project for learning purposes only. Feel free to use it.
 *
 * JuraszekL
 *
 * */

//***************************************

#ifndef BME280_H
#define BME280_H

//***************************************

#ifdef __cplusplus /* CPP */
extern "C" {
#endif

//***************************************

#include "bme280_definitions.h"

//***************************************



//***************************************
/* public functions */
//***************************************

int8_t BME280_Init(BME280_t *Dev, uint8_t I2cAddr, void *EnvSpecData,
		bme280_readbytes ReadFun, bme280_writebyte WriteFun);

int8_t BME280_ConfigureAll(BME280_t *Dev, BME280_Config_t *Config);

int8_t BME280_GetMode(BME280_t *Dev, uint8_t *Mode);
int8_t BME280_SetMode(BME280_t *Dev, uint8_t Mode);

int8_t BME280_GetPOvs(BME280_t *Dev, uint8_t *POvs);
int8_t BME280_SetPOvs(BME280_t *Dev, uint8_t POvs);

int8_t BME280_GetTOvs(BME280_t *Dev, uint8_t *TOvs);
int8_t BME280_SetTOvs(BME280_t *Dev, uint8_t TOvs);

//***************************************

#ifdef __cplusplus
}
#endif /* CPP */

//***************************************

#endif /* BME280_H */
