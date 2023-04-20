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
#include <stddef.h>

//***************************************
/* returned values */
//***************************************

#define BME280_OK	(0)
#define BME280_ERR	(-1)

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

#define BME280_ID	(0x60)

//***************************************
/* registers values */
//***************************************

typedef enum {

	i2c = 1U,
	spi

}BME280_InterfaceType_t;

//***************************************
/* device structure */
//***************************************

typedef struct {

	BME280_InterfaceType_t interface;
	uint8_t i2c_address;

}BME280_Device_t;

#endif /* BME280_DEFINITIONS_H */
