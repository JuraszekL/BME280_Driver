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
/* returned values */
//***************************************

#define BME280_OK				(0)
#define BME280_ERR				(-1)
#define BME280_INTERFACE_ERR	(-2)
#define BME280_ID_ERROR			(-3)

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

#define BME280_ID			(0x60)
#define BME280_REGADDR_ID	(0xD0)

//***************************************
/* typedefs */
//***************************************

//typedef enum {bme280_i2c = 1U, bme280_spi} BME280_InterfaceType_t;

typedef int8_t (*bme280_readbytes)(uint8_t reg_addr, uint8_t *rxbuff, uint8_t rxlen, uint8_t dev_addr, void *env_spec_data);
typedef int8_t (*bme280_writebyte)(uint8_t reg_addr, uint8_t value, uint8_t dev_addr, void *env_spec_data);


typedef struct {

//	BME280_InterfaceType_t interface;
	uint8_t i2c_address;
	void *env_spec_data;

	bme280_readbytes read;
	bme280_writebyte write;

}BME280_Dev_t;


#endif /* BME280_DEFINITIONS_H */
