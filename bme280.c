
//***************************************

#include <stdint.h>
#include <stddef.h>
#include "bme280.h"

//***************************************








//***************************************
/* static functions declarations */
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
	if((NULL == ReadFun) || (NULL == WriteFun) || (NULL == Dev)) return BME280_PARAM_ERR;

	Dev->i2c_address = I2cAddr;
	Dev->env_spec_data = EnvSpecData;
	Dev->read = ReadFun;
	Dev->write = WriteFun;

	/* read and check chip ID */
	res = Dev->read(BME280_ID_ADDR, &id, 1, Dev->i2c_address, Dev->env_spec_data);
	if(BME280_OK != res) return BME280_INTERFACE_ERR;

	if(BME280_ID != id) return BME280_ID_ERR;

	res = BME280_Reset(Dev);
	if(BME280_OK != res) return res;

	/* read, parse and store compensation data */
	res = bme280_read_compensation_parameters(Dev);

	if(BME280_OK == res) Dev->initialized = BME280_INITIALIZED;
	return res;
}

	/* Function  configures all sensor parameters at once */
int8_t BME280_ConfigureAll(BME280_t *Dev, BME280_Config_t *Config){

	int8_t res = BME280_OK;
	uint8_t ctrl_hum = 0, ctrl_meas = 0, config = 0;

	/* check parameters */
	if((NULL == Dev) || (NULL == Config)) return BME280_PARAM_ERR;

	/* check if sensor has been initialized before */
	if(BME280_NOT_INITIALIZED == Dev->initialized) return BME280_NO_INIT_ERR;

	/* set the data from Config structure to the right positions in
	 * sensor registers */
	ctrl_hum = Config->oversampling_h & 0x07;	//0x07 - 0b00000111

	ctrl_meas |= (Config->oversampling_t << 5) & 0xE0; 	//0xE0 - 0b11100000
	ctrl_meas |= (Config->oversampling_p << 2) & 0x1C;	//0x1C - 0b00011100
	ctrl_meas |= Config->mode & 0x03;					//0x03 - 0b00000011

	config |= (Config->t_stby << 5) & 0xE0;	//0xE0 - 0b11100000
	config |= (Config->filter << 2) & 0x1C;	//0x1C - 0b00011100
	config |= Config->spi3w_enable & 0x01;	//0x01 - 0b00000001

	/* send three config bytes to the device */
	res = Dev->write(BME280_CTRL_HUM_ADDR, ctrl_hum, Dev->i2c_address, Dev->env_spec_data);
	if(BME280_OK != res) return BME280_INTERFACE_ERR;
	res = Dev->write(BME280_CTRL_MEAS_ADDR, ctrl_meas, Dev->i2c_address, Dev->env_spec_data);
	if(BME280_OK != res) return BME280_INTERFACE_ERR;
	res = Dev->write(BME280_CONFIG_ADDR, config, Dev->i2c_address, Dev->env_spec_data);
	if(BME280_OK != res) return BME280_INTERFACE_ERR;

	return res;
}

	/* function performs power-on reset procedure for sensor */
int8_t BME280_Reset(BME280_t *Dev){

	int8_t res = BME280_OK;

	/* check parameter */
	if((NULL == Dev) || (NULL == Dev->write)) return BME280_PARAM_ERR;

	/* write reset commad to reset register */
	res = Dev->write(BME280_RESET_ADDR, BME280_RESET_VALUE, Dev->i2c_address, Dev->env_spec_data);

	return res;
}

	/* Function reads current operation mode from sensor */
int8_t BME280_GetMode(BME280_t *Dev, uint8_t *Mode){

	int8_t res = BME280_OK;
	uint8_t ctrl_meas = 0;

	/* check parameters */
	if((NULL == Dev) || (NULL == Mode)) return BME280_PARAM_ERR;

	/* check if sensor has been initialized before */
	if(BME280_NOT_INITIALIZED == Dev->initialized) return BME280_NO_INIT_ERR;

	/* read value of ctrl_meas register from sensor */
	res = Dev->read(BME280_CTRL_MEAS_ADDR, &ctrl_meas, 1, Dev->i2c_address, Dev->env_spec_data);
	if(BME280_OK != res) return BME280_INTERFACE_ERR;

	/* parse mode values from ctrl_meas */
	ctrl_meas &= 0x03;
	if(0x02 == ctrl_meas) ctrl_meas = BME280_FORCEDMODE;

	/* set output pointer */
	*Mode = ctrl_meas;

	return res;
}

	/* Function sets sensor operation mode */
int8_t BME280_SetMode(BME280_t *Dev, uint8_t Mode){

	int8_t res = BME280_OK;
	uint8_t ctrl_meas = 0, tmp = 0;

	/* check parameters */
	if((NULL == Dev) || (Mode > BME280_NORMALMODE)) return BME280_PARAM_ERR;

	/* check if sensor has been initialized before */
	if(BME280_NOT_INITIALIZED == Dev->initialized) return BME280_NO_INIT_ERR;

	/* read value of ctrl_meas register from sensor */
	res = Dev->read(BME280_CTRL_MEAS_ADDR, &ctrl_meas, 1, Dev->i2c_address, Dev->env_spec_data);
	if(BME280_OK != res) return BME280_INTERFACE_ERR;

	/* check if current mode differs from requested */
	tmp = ctrl_meas & 0x03;
	if(0x02 == tmp) tmp = BME280_FORCEDMODE;
	if(Mode == tmp) return BME280_OK;

	/* send new ctrl_meas value to sensor if required */
	ctrl_meas &= 0xFC;	//0xFC - 0b11111100
	ctrl_meas |= Mode;
	res = Dev->write(BME280_CTRL_MEAS_ADDR, ctrl_meas, Dev->i2c_address, Dev->env_spec_data);

	return res;
}

	/* function gets current pressure oversampling value from sensor */
int8_t BME280_GetPOvs(BME280_t *Dev, uint8_t *POvs){

	int8_t res = BME280_OK;
	uint8_t ctrl_meas = 0;

	/* check parameters */
	if((NULL == Dev) || (NULL == POvs)) return BME280_PARAM_ERR;

	/* check if sensor has been initialized before */
	if(BME280_NOT_INITIALIZED == Dev->initialized) return BME280_NO_INIT_ERR;

	/* read value of ctrl_meas register from sensor */
	res = Dev->read(BME280_CTRL_MEAS_ADDR, &ctrl_meas, 1, Dev->i2c_address, Dev->env_spec_data);
	if(BME280_OK != res) return BME280_INTERFACE_ERR;

	/* parse pressure oversampling value from ctrl_meas */
	ctrl_meas = (ctrl_meas >> 2) & 0x07;

	/* set output pointer */
	*POvs = ctrl_meas;

	return res;
}

	/* function sets pressure oversampling value  */
int8_t BME280_SetPOvs(BME280_t *Dev, uint8_t POvs){

	int8_t res = BME280_OK;
	uint8_t ctrl_meas = 0, tmp = 0, mode = 0;

	/* check parameters */
	if((NULL == Dev) || (POvs > BME280_OVERSAMPLING_X16)) return BME280_PARAM_ERR;

	/* check if sensor has been initialized before */
	if(BME280_NOT_INITIALIZED == Dev->initialized) return BME280_NO_INIT_ERR;

	/* check if sensor is in sleep mode */
	res = BME280_GetMode(Dev, &mode);
	if(BME280_OK != res) return res;
	if(BME280_SLEEPMODE != mode) return BME280_CONDITION_ERR;

	/* read value of ctrl_meas register from sensor */
	res = Dev->read(BME280_CTRL_MEAS_ADDR, &ctrl_meas, 1, Dev->i2c_address, Dev->env_spec_data);
	if(BME280_OK != res) return BME280_INTERFACE_ERR;

	/* check if current value differs from requested */
	tmp = (ctrl_meas >> 2) & 0x07;
	if(POvs == tmp) return BME280_OK;

	/* send new ctrl_meas value to sensor if required */
	ctrl_meas &= 0xE3;	//0xE3 - 0b11100011
	ctrl_meas |= (POvs << 2);
	res = Dev->write(BME280_CTRL_MEAS_ADDR, ctrl_meas, Dev->i2c_address, Dev->env_spec_data);

	return res;
}

/* function gets current temperature oversampling value from sensor */
int8_t BME280_GetTOvs(BME280_t *Dev, uint8_t *TOvs){

	int8_t res = BME280_OK;
	uint8_t ctrl_meas = 0;

	/* check parameters */
	if((NULL == Dev) || (NULL == TOvs)) return BME280_PARAM_ERR;

	/* check if sensor has been initialized before */
	if(BME280_NOT_INITIALIZED == Dev->initialized) return BME280_NO_INIT_ERR;

	/* read value of ctrl_meas register from sensor */
	res = Dev->read(BME280_CTRL_MEAS_ADDR, &ctrl_meas, 1, Dev->i2c_address, Dev->env_spec_data);
	if(BME280_OK != res) return BME280_INTERFACE_ERR;

	/* parse temperature oversampling value from ctrl_meas */
	ctrl_meas = (ctrl_meas >> 5) & 0x07;

	/* set output pointer */
	*TOvs = ctrl_meas;

	return res;
}

/* function sets temperature oversampling value  */
int8_t BME280_SetTOvs(BME280_t *Dev, uint8_t TOvs){

	int8_t res = BME280_OK;
	uint8_t ctrl_meas = 0, tmp = 0, mode = 0;

	/* check parameters */
	if((NULL == Dev) || (TOvs > BME280_OVERSAMPLING_X16)) return BME280_PARAM_ERR;

	/* check if sensor has been initialized before */
	if(BME280_NOT_INITIALIZED == Dev->initialized) return BME280_NO_INIT_ERR;

	/* check if sensor is in sleep mode */
	res = BME280_GetMode(Dev, &mode);
	if(BME280_OK != res) return res;
	if(BME280_SLEEPMODE != mode) return BME280_CONDITION_ERR;

	/* read value of ctrl_meas register from sensor */
	res = Dev->read(BME280_CTRL_MEAS_ADDR, &ctrl_meas, 1, Dev->i2c_address, Dev->env_spec_data);
	if(BME280_OK != res) return BME280_INTERFACE_ERR;

	/* check if current value differs from requested */
	tmp = (ctrl_meas >> 5) & 0x07;
	if(TOvs == tmp) return BME280_OK;

	/* send new ctrl_meas value to sensor if required */
	ctrl_meas &= 0x1F;	//0x1F - 0b00011111
	ctrl_meas |= (TOvs << 5);
	res = Dev->write(BME280_CTRL_MEAS_ADDR, ctrl_meas, Dev->i2c_address, Dev->env_spec_data);

	return res;
}

/* function gets current humidity oversampling value from sensor */
int8_t BME280_GetHOvs(BME280_t *Dev, uint8_t *HOvs){

	int8_t res = BME280_OK;
	uint8_t ctrl_hum = 0;

	/* check parameters */
	if((NULL == Dev) || (NULL == HOvs)) return BME280_PARAM_ERR;

	/* check if sensor has been initialized before */
	if(BME280_NOT_INITIALIZED == Dev->initialized) return BME280_NO_INIT_ERR;

	/* read value of ctrl_hum register from sensor */
	res = Dev->read(BME280_CTRL_HUM_ADDR, &ctrl_hum, 1, Dev->i2c_address, Dev->env_spec_data);
	if(BME280_OK != res) return BME280_INTERFACE_ERR;

	/* parse humidity oversampling value from ctrl_hum */
	ctrl_hum = ctrl_hum & 0x07;

	/* set output pointer */
	*HOvs = ctrl_hum;

	return res;
}

/* function sets humidity oversampling value  */
int8_t BME280_SetHOvs(BME280_t *Dev, uint8_t HOvs){

	int8_t res = BME280_OK;
	uint8_t tmp = 0, mode = 0;

	/* check parameters */
	if((NULL == Dev) || (HOvs > BME280_OVERSAMPLING_X16)) return BME280_PARAM_ERR;

	/* check if sensor has been initialized before */
	if(BME280_NOT_INITIALIZED == Dev->initialized) return BME280_NO_INIT_ERR;

	/* check if sensor is in sleep mode */
	res = BME280_GetMode(Dev, &mode);
	if(BME280_OK != res) return res;
	if(BME280_SLEEPMODE != mode) return BME280_CONDITION_ERR;

	/* send requested value to sensor */
	res = Dev->write(BME280_CTRL_HUM_ADDR, HOvs, Dev->i2c_address, Dev->env_spec_data);
	if(BME280_OK != res) return BME280_INTERFACE_ERR;

	/* to make the change effective we need to write ctrl_meas register,
	 * check documentation */
	res = Dev->read(BME280_CTRL_MEAS_ADDR, &tmp, 1, Dev->i2c_address, Dev->env_spec_data);
	if(BME280_OK != res) return BME280_INTERFACE_ERR;
	res = Dev->write(BME280_CTRL_MEAS_ADDR, tmp, Dev->i2c_address, Dev->env_spec_data);

	return res;
}

	/* function gets current standby time for normal mode */
int8_t BME280_GetTStby(BME280_t *Dev, uint8_t *TStby){

	int8_t res = BME280_OK;
	uint8_t config = 0;

	/* check parameters */
	if((NULL == Dev) || (NULL == TStby)) return BME280_PARAM_ERR;

	/* check if sensor has been initialized before */
	if(BME280_NOT_INITIALIZED == Dev->initialized) return BME280_NO_INIT_ERR;

	/* read value of config register from sensor */
	res = Dev->read(BME280_CONFIG_ADDR, &config, 1, Dev->i2c_address, Dev->env_spec_data);
	if(BME280_OK != res) return BME280_INTERFACE_ERR;

	/* parse standby time value from config */
	config = (config >> 5) & 0x07;

	/* set output pointer */
	*TStby = config;

	return res;
}

	/* function sets standby time */
int8_t BME280_SetTStby(BME280_t *Dev, uint8_t TStby){

	int8_t res = BME280_OK;
	uint8_t config = 0, tmp = 0, mode = 0;

	/* check parameters */
	if((NULL == Dev) || (TStby > BME280_STBY_20MS)) return BME280_PARAM_ERR;

	/* check if sensor has been initialized before */
	if(BME280_NOT_INITIALIZED == Dev->initialized) return BME280_NO_INIT_ERR;

	/* check if sensor is in sleep mode */
	res = BME280_GetMode(Dev, &mode);
	if(BME280_OK != res) return res;
	if(BME280_SLEEPMODE != mode) return BME280_CONDITION_ERR;

	/* read value of config register from sensor */
	res = Dev->read(BME280_CONFIG_ADDR, &config, 1, Dev->i2c_address, Dev->env_spec_data);
	if(BME280_OK != res) return BME280_INTERFACE_ERR;

	/* check if current value differs from requested */
	tmp = (config >> 5) & 0x07;
	if(TStby == tmp) return BME280_OK;

	/* send new config value to sensor if required */
	config &= 0x1F;	//0x1F - 0b00011111
	config |= (TStby << 5);
	res = Dev->write(BME280_CONFIG_ADDR, config, Dev->i2c_address, Dev->env_spec_data);

	return res;
}

	/* function gets current value of IIR filter */
int8_t BME280_GetTFilter(BME280_t *Dev, uint8_t *Filter){

	int8_t res = BME280_OK;
	uint8_t config = 0;

	/* check parameters */
	if((NULL == Dev) || (NULL == Filter)) return BME280_PARAM_ERR;

	/* check if sensor has been initialized before */
	if(BME280_NOT_INITIALIZED == Dev->initialized) return BME280_NO_INIT_ERR;

	/* read value of config register from sensor */
	res = Dev->read(BME280_CONFIG_ADDR, &config, 1, Dev->i2c_address, Dev->env_spec_data);
	if(BME280_OK != res) return BME280_INTERFACE_ERR;

	/* parse filter value from config */
	config = (config >> 2) & 0x07;

	/* set output pointer */
	*Filter = config;

	return res;
}

	/* function sets IIR filter value */
int8_t BME280_SetFilter(BME280_t *Dev, uint8_t Filter){

	int8_t res = BME280_OK;
	uint8_t config = 0, tmp = 0, mode = 0;

	/* check parameters */
	if((NULL == Dev) || (Filter > BME280_FILTER_16)) return BME280_PARAM_ERR;

	/* check if sensor has been initialized before */
	if(BME280_NOT_INITIALIZED == Dev->initialized) return BME280_NO_INIT_ERR;

	/* check if sensor is in sleep mode */
	res = BME280_GetMode(Dev, &mode);
	if(BME280_OK != res) return res;
	if(BME280_SLEEPMODE != mode) return BME280_CONDITION_ERR;

	/* read value of config register from sensor */
	res = Dev->read(BME280_CONFIG_ADDR, &config, 1, Dev->i2c_address, Dev->env_spec_data);
	if(BME280_OK != res) return BME280_INTERFACE_ERR;

	/* check if current value differs from requested */
	tmp = (config >> 2) & 0x07;
	if(Filter == tmp) return BME280_OK;

	/* send new config value to sensor if required */
	config &= 0xE3;	//0xE3 - 0b11100011
	config |= (Filter << 2);
	res = Dev->write(BME280_CONFIG_ADDR, config, Dev->i2c_address, Dev->env_spec_data);

	return res;
}

	/* function enables 3-wire SPI interface */
int8_t BME280_Enable3WireSPI(BME280_t *Dev){

	int8_t res = BME280_OK;
	uint8_t config = 0, tmp = 0, mode = 0;

	/* check parameter */
	if(NULL == Dev) return BME280_PARAM_ERR;

	/* check if sensor has been initialized before */
	if(BME280_NOT_INITIALIZED == Dev->initialized) return BME280_NO_INIT_ERR;

	/* check if sensor is in sleep mode */
	res = BME280_GetMode(Dev, &mode);
	if(BME280_OK != res) return res;
	if(BME280_SLEEPMODE != mode) return BME280_CONDITION_ERR;

	/* read value of config register from sensor */
	res = Dev->read(BME280_CONFIG_ADDR, &config, 1, Dev->i2c_address, Dev->env_spec_data);
	if(BME280_OK != res) return BME280_INTERFACE_ERR;

	/* check if current value differs from requested */
	tmp = config & 0x01;
	if(0x01 == tmp) return BME280_OK;

	/* send new config value to sensor if required */
	config &= 0xFE;	//0xFE - 0b11111110
	config |= 0x01;
	res = Dev->write(BME280_CONFIG_ADDR, config, Dev->i2c_address, Dev->env_spec_data);

	return res;
}

/* function disables 3-wire SPI interface */
int8_t BME280_Disable3WireSPI(BME280_t *Dev){

	int8_t res = BME280_OK;
	uint8_t config = 0, tmp = 0, mode = 0;

	/* check parameter */
	if(NULL == Dev) return BME280_PARAM_ERR;

	/* check if sensor has been initialized before */
	if(BME280_NOT_INITIALIZED == Dev->initialized) return BME280_NO_INIT_ERR;

	/* check if sensor is in sleep mode */
	res = BME280_GetMode(Dev, &mode);
	if(BME280_OK != res) return res;
	if(BME280_SLEEPMODE != mode) return BME280_CONDITION_ERR;

	/* read value of config register from sensor */
	res = Dev->read(BME280_CONFIG_ADDR, &config, 1, Dev->i2c_address, Dev->env_spec_data);
	if(BME280_OK != res) return BME280_INTERFACE_ERR;

	/* check if current value differs from requested */
	tmp = config & 0x01;
	if(0x00 == tmp) return BME280_OK;

	/* send new config value to sensor if required */
	config &= 0xFE;	//0xFE - 0b11111110
	res = Dev->write(BME280_CONFIG_ADDR, config, Dev->i2c_address, Dev->env_spec_data);

	return res;
}

	/* function reads current 3-wire SPI setup (0 = 3w SPI disabled, 1 - 3w SPI enabled) */
int8_t BME280_Is3WireSPIEnabled(BME280_t *Dev, uint8_t *Result){

	int8_t res = BME280_OK;
	uint8_t config = 0;

	/* check parameters */
	if((NULL == Dev) || (NULL == Result)) return BME280_PARAM_ERR;

	/* check if sensor has been initialized before */
	if(BME280_NOT_INITIALIZED == Dev->initialized) return BME280_NO_INIT_ERR;

	/* read value of ctrl_meas register from sensor */
	res = Dev->read(BME280_CONFIG_ADDR, &config, 1, Dev->i2c_address, Dev->env_spec_data);
	if(BME280_OK != res) return BME280_INTERFACE_ERR;

	/* parse mode values from ctrl_meas */
	config &= 0x01;

	/* set output pointer */
	*Result = config;

	return res;
}

//***************************************
/* static functions */
//***************************************

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
