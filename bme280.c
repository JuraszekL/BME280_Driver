
//***************************************

#include <stdint.h>
#include "bme280.h"

//***************************************



//***************************************
/* public functions */
//***************************************

int8_t BME280_Init(BME280_Dev_t *dev, uint8_t i2c_addr){

	int8_t res = BME280_OK;

	dev->i2c_address = i2c_addr;

	return res;
}
