
//***************************************

#include <stdint.h>
#include <stddef.h>
#include "bme280.h"

//***************************************








//***************************************
/* static declarations */
//***************************************

static int8_t bme280_read_compensation_parameters(BME280_t *Dev);

//***************************************
/* public functions */
//***************************************

int8_t BME280_Init(BME280_t *Dev, uint8_t I2cAddr, void *EnvSpecData,
		bme280_readbytes ReadFun, bme280_writebyte WriteFun){

	int8_t res = BME280_OK;
	uint8_t id = 0;

	/* check parameters */
	if((NULL == ReadFun) || (NULL == WriteFun)) return BME280_ERR;

	Dev->i2c_address = I2cAddr;
	Dev->env_spec_data = EnvSpecData;
	Dev->read = ReadFun;
	Dev->write = WriteFun;

	/* read and check chip ID */
	res = Dev->read(BME280_REGADDR_ID, &id, 1, Dev->i2c_address, Dev->env_spec_data);
	if(BME280_OK != res) return BME280_INTERFACE_ERR;

	if(BME280_ID != id) return BME280_ID_ERROR;

	/* read, parse and store compensation data */
	res = bme280_read_compensation_parameters(Dev);
	if(BME280_OK != res) return res;

	return res;
}




static int8_t bme280_read_compensation_parameters(BME280_t *Dev){

	uint8_t tmp_buff[32];
	int8_t res;

	/* read two calibration data's areas from sensor */
	res = Dev->read(BME280_CALIB_DATA1_ADDR, &tmp_buff[0], BME280_CALIB_DATA1_LEN, Dev->i2c_address, Dev->env_spec_data);
	if(BME280_OK != res) return res;
	res = Dev->read(BME280_CALIB_DATA2_ADDR, &tmp_buff[24], BME280_CALIB_DATA2_LEN, Dev->i2c_address, Dev->env_spec_data);
	if(BME280_OK != res) return res;

	// parse data to the structure inside Dev

	return 0;
}
