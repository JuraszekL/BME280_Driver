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
/* driver config */
//***************************************

	/* comment this line if you don't want to use
	 * 64bit variables in calculations */
#define USE_64BIT

//***************************************
/* public functions */
//***************************************

int8_t BME280_Init(BME280_t *Dev, uint8_t I2cAddr, void *EnvSpecData,
		bme280_readbytes ReadFun, bme280_writebyte WriteFun, bme280_delayms Delay);

int8_t BME280_ConfigureAll(BME280_t *Dev, BME280_Config_t *Config);

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

//***************************************

#ifdef __cplusplus
}
#endif /* CPP */

//***************************************

#endif /* BME280_H */
