
//***************************************

#include <stdint.h>
#include <stddef.h>
#include "bme280.h"

//***************************************








//***************************************
/* static declarations */
//***************************************

	/* private function to read compensation parameters from sensor and
	 * parse them inside  BME280_t structure */
static int8_t bme280_read_compensation_parameters(BME280_t *Dev);

//***************************************
/* public functions */
//***************************************

	/* function that initiates minimum required parameters to operate
	 * a sensor */
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



	/* private function to read compensation parameters from sensor and
	 * parse them inside  BME280_t structure */
static int8_t bme280_read_compensation_parameters(BME280_t *Dev){

	uint8_t tmp_buff[32];
	int8_t res;

	/* read two calibration data's areas from sensor */
	res = Dev->read(BME280_CALIB_DATA1_ADDR, &tmp_buff[0], BME280_CALIB_DATA1_LEN, Dev->i2c_address, Dev->env_spec_data);
	if(BME280_OK != res) return res;
	res = Dev->read(BME280_CALIB_DATA2_ADDR, &tmp_buff[25], BME280_CALIB_DATA2_LEN, Dev->i2c_address, Dev->env_spec_data);
	if(BME280_OK != res) return res;

	// parse data to the structure inside Dev
	Dev->trimm.dig_T1 = CAT_UI16T(tmp_buff[1], tmp_buff[0]);
	Dev->trimm.dig_T2 = CAT_I16T(tmp_buff[3], tmp_buff[2]);
	Dev->trimm.dig_T3 = CAT_I16T(tmp_buff[5], tmp_buff[4]);

	Dev->trimm.dig_P1 = CAT_UI16T(tmp_buff[7], tmp_buff[6]);
	Dev->trimm.dig_P2 = CAT_I16T(tmp_buff[9], tmp_buff[8]);
	Dev->trimm.dig_P3 = CAT_I16T(tmp_buff[11], tmp_buff[10]);
	Dev->trimm.dig_P4 = CAT_I16T(tmp_buff[13], tmp_buff[12]);
	Dev->trimm.dig_P5 = CAT_I16T(tmp_buff[15], tmp_buff[14]);
	Dev->trimm.dig_P6 = CAT_I16T(tmp_buff[17], tmp_buff[16]);
	Dev->trimm.dig_P7 = CAT_I16T(tmp_buff[19], tmp_buff[18]);
	Dev->trimm.dig_P8 = CAT_I16T(tmp_buff[21], tmp_buff[20]);
	Dev->trimm.dig_P9 = CAT_I16T(tmp_buff[23], tmp_buff[22]);

	Dev->trimm.dig_H1 = tmp_buff[24];
	Dev->trimm.dig_H2 = CAT_I16T(tmp_buff[26], tmp_buff[25]);
	Dev->trimm.dig_H3 = tmp_buff[27];
						/*       	MSB              				LSB			       */
	Dev->trimm.dig_H4 = ( ((int16_t)tmp_buff[28] << 4) | ((int16_t)tmp_buff[29] & 0x0F) );

						/*       	MSB              				LSB			       */
	Dev->trimm.dig_H5 = ( ((int16_t)tmp_buff[30] << 4) | ((int16_t)tmp_buff[29] >> 4) );
	Dev->trimm.dig_H6 = (int8_t)tmp_buff[31];

	return BME280_OK;
}
