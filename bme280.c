
//***************************************

#include <stdint.h>
#include <stddef.h>
#include "bme280.h"

//***************************************
/* private types and structures */
//***************************************

	/* concatenate two bytes into signed half-word */
#define CAT_I16T(msb, lsb) ((int16_t)(((int16_t)msb << 8) | (int16_t)lsb))
	/* concatenate two bytes into unsigned half-word */
#define CAT_UI16T(msb, lsb) ((uint16_t)(((uint16_t)msb << 8) | (uint16_t)lsb))

	/* check if x is null */
#define IS_NULL(x)	((NULL == x))

	/* type of read */
enum { read_all = 0, read_temp, read_press, read_hum};

	/* possible value of mode variable inside BME280_t structure */
enum {sleep_mode = 0x00, forced_mode = 0x01, normal_mode = 0x03};

__attribute__((aligned(1))) struct adc_regs {

	uint8_t press_raw[BME280_PRESS_ADC_LEN];
	uint8_t temp_raw[BME280_TEMP_ADC_LEN];
	uint8_t hum_raw[BME280_HUM_ADC_LEN];

};

//***************************************
/* static functions declarations */
//***************************************

	/* private function to read compensation parameters from sensor and
	 * parse them inside  BME280_t structure */
static int8_t bme280_read_compensation_parameters(BME280_t *Dev);

	/* private function to read and compensate selected adc
	 * data from sensor  */
static int8_t bme280_read_compensate(uint8_t read_type, BME280_t *Dev, BME280_S32_t *temp,
	BME280_U32_t *press, BME280_U32_t *hum);

	/* private function that parses raw adc pressure or temp values
	 * from sensor into a single BME280_S32_t variable */
static BME280_S32_t bme280_parse_press_temp_s32t(uint8_t *raw);

	/* private function that parses raw adc humidity values
	 * from sensor into a single BME280_S32_t variable */
static BME280_S32_t bme280_parse_hum_s32t(uint8_t *raw);

	/* Returns temperature in DegC, resolution is 0.01 DegC. Output value of “5123”
	 * equals 51.23 DegC. t_fine carries fine temperature as global value */
static BME280_S32_t bme280_compensate_t_s32t(BME280_t *Dev, BME280_S32_t adc_T);

	/* Returns pressure in Pa as unsigned 32 bit integer in Q24.8 format
	 * (24 integer bits and 8 fractional bits). Output value of “24674867”
	 * represents 24674867/256 = 96386.2 Pa = 963.862 hPa */
static BME280_U32_t bme280_compensate_p_u32t(BME280_t *Dev, BME280_S32_t adc_P);

	/* Returns humidity in %RH as unsigned 32bit integer in Q22.10 format (22 integer
	 * and 10 fractional bits). Output value of "47445" represents 47445/1024 = 46.333 %RH */
static BME280_U32_t bme280_compensate_h_u32t(BME280_t *Dev, BME280_S32_t adc_H);

	/* function converts BME280_S32_t temperature to BME280_Data_t structure */
static void bme280_convert_t_S32_struct(BME280_S32_t temp, BME280_Data_t *data);

	/* function converts BME280_S32_t temperature to float  */
static void bme280_convert_t_S32_float(BME280_S32_t temp_in, float *temp_out);

	/* function converts BME280_S32_t pressure to BME280_Data_t structure */
static void bme280_convert_p_U32_struct(BME280_U32_t press, BME280_Data_t *data);

	/* function converts BME280_U32_t pressure to float */
static void bme280_convert_p_U32_float(BME280_U32_t press_in, float *press_out);

	/* function converts BME280_U32_t humidity to BME280_Data_t structure */
static void bme280_convert_h_U32_struct(BME280_U32_t hum, BME280_Data_t *data);

	/* function converts BME280_U32_t humidity to float */
static void bme280_convert_h_U32_float(BME280_U32_t hum_in, float *hum_out);

	/* function checks if device was initialized and is in normal mode */
static int8_t bme280_is_normal_mode(BME280_t *Dev);

	/* function checks if device was initialized and is in sleep mode */
static int8_t bme280_is_sleep_mode(BME280_t *Dev);

//***************************************
/* public functions */
//***************************************

	/* function that initiates minimum required parameters to operate
	 * a sensor */
int8_t BME280_Init(BME280_t *Dev, uint8_t I2cAddr, void *EnvSpecData,
		bme280_readbytes ReadFun, bme280_writebyte WriteFun, bme280_delayms Delay){

	int8_t res = BME280_OK;
	uint8_t id = 0;

	/* check parameters */
	if( IS_NULL(ReadFun) || IS_NULL(WriteFun) ||IS_NULL(Dev) ||
			IS_NULL(Delay) ) return BME280_PARAM_ERR;

	/* fill the structure */
	Dev->i2c_address = I2cAddr;
	Dev->env_spec_data = EnvSpecData;
	Dev->read = ReadFun;
	Dev->write = WriteFun;
	Dev->delay = Delay;

	/* perform sensor reset */
	res = BME280_Reset(Dev);
	if(BME280_OK != res) return res;

	/* Start-up time = 2ms */
	Dev->delay(2);

	/* read and check chip ID */
	res = Dev->read(BME280_ID_ADDR, &id, 1, Dev->i2c_address, Dev->env_spec_data);
	if(BME280_OK != res) return BME280_INTERFACE_ERR;

	if(BME280_ID != id) return BME280_ID_ERR;

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
	if( IS_NULL(Dev) || IS_NULL(Config) ) return BME280_PARAM_ERR;

	/* check if sensor is initialized and in sleep mode */
	res = bme280_is_sleep_mode(Dev);
	if(BME280_OK != res) return res;

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
	if( IS_NULL(Dev) || IS_NULL(Dev->write) ) return BME280_PARAM_ERR;

	/* write reset commad to reset register */
	res = Dev->write(BME280_RESET_ADDR, BME280_RESET_VALUE, Dev->i2c_address, Dev->env_spec_data);

	/* set mode to default */
	Dev->mode = sleep_mode;

	return res;
}

	/* Function reads current operation mode from sensor */
int8_t BME280_GetMode(BME280_t *Dev, uint8_t *Mode){

	int8_t res = BME280_OK;
	uint8_t ctrl_meas = 0;

	/* check parameters */
	if( IS_NULL(Dev) || IS_NULL(Mode) ) return BME280_PARAM_ERR;

	/* check if sensor is initialized */
	res = bme280_is_sleep_mode(Dev);
	if(BME280_NO_INIT_ERR == res) return res;

	/* read value of ctrl_meas register from sensor */
	res = Dev->read(BME280_CTRL_MEAS_ADDR, &ctrl_meas, 1, Dev->i2c_address, Dev->env_spec_data);
	if(BME280_OK != res) return BME280_INTERFACE_ERR;

	/* parse mode values from ctrl_meas */
	ctrl_meas &= 0x03;
	if(0x02 == ctrl_meas) ctrl_meas = BME280_FORCEDMODE;

	/* update value inside Dev structure */
	Dev->mode = ctrl_meas;

	/* set output pointer */
	*Mode = ctrl_meas;

	return res;
}

	/* Function sets sensor operation mode */
int8_t BME280_SetMode(BME280_t *Dev, uint8_t Mode){

	int8_t res = BME280_OK;
	uint8_t ctrl_meas = 0, tmp = 0;

	/* check parameters */
	if( IS_NULL(Dev) || (Mode > BME280_NORMALMODE) ) return BME280_PARAM_ERR;

	/* check if sensor is initialized */
	res = bme280_is_sleep_mode(Dev);
	if(BME280_NO_INIT_ERR == res) return res;

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
	if(BME280_OK != res) return BME280_INTERFACE_ERR;

	/* update value inside Dev structure */
	Dev->mode = Mode;

	return res;
}

	/* function gets current pressure oversampling value from sensor */
int8_t BME280_GetPOvs(BME280_t *Dev, uint8_t *POvs){

	int8_t res = BME280_OK;
	uint8_t ctrl_meas = 0;

	/* check parameters */
	if( IS_NULL(Dev) || IS_NULL(POvs) ) return BME280_PARAM_ERR;

	/* check if sensor is initialized */
	res = bme280_is_sleep_mode(Dev);
	if(BME280_NO_INIT_ERR == res) return res;

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
	uint8_t ctrl_meas = 0, tmp = 0;

	/* check parameters */
	if( IS_NULL(Dev) || (POvs > BME280_OVERSAMPLING_X16) ) return BME280_PARAM_ERR;

	/* check if sensor is initialized and in sleep mode */
	res = bme280_is_sleep_mode(Dev);
	if(BME280_OK != res) return res;

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
	if( IS_NULL(Dev) || IS_NULL(TOvs) ) return BME280_PARAM_ERR;

	/* check if sensor is initialized */
	res = bme280_is_sleep_mode(Dev);
	if(BME280_NO_INIT_ERR == res) return res;

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
	uint8_t ctrl_meas = 0, tmp = 0;

	/* check parameters */
	if( IS_NULL(Dev) || (TOvs > BME280_OVERSAMPLING_X16) ) return BME280_PARAM_ERR;

	/* check if sensor is initialized and in sleep mode */
	res = bme280_is_sleep_mode(Dev);
	if(BME280_OK != res) return res;

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
	if( IS_NULL(Dev) || IS_NULL(HOvs) ) return BME280_PARAM_ERR;

	/* check if sensor is initialized */
	res = bme280_is_sleep_mode(Dev);
	if(BME280_NO_INIT_ERR == res) return res;

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
	uint8_t tmp = 0;

	/* check parameters */
	if( IS_NULL(Dev) || (HOvs > BME280_OVERSAMPLING_X16) ) return BME280_PARAM_ERR;

	/* check if sensor is initialized and in sleep mode */
	res = bme280_is_sleep_mode(Dev);
	if(BME280_OK != res) return res;

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
	if( IS_NULL(Dev) || IS_NULL(TStby) ) return BME280_PARAM_ERR;

	/* check if sensor is initialized */
	res = bme280_is_sleep_mode(Dev);
	if(BME280_NO_INIT_ERR == res) return res;

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
	uint8_t config = 0, tmp = 0;

	/* check parameters */
	if( IS_NULL(Dev) || (TStby > BME280_STBY_20MS) ) return BME280_PARAM_ERR;

	/* check if sensor is initialized and in sleep mode */
	res = bme280_is_sleep_mode(Dev);
	if(BME280_OK != res) return res;

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
	if( IS_NULL(Dev) || IS_NULL(Filter) ) return BME280_PARAM_ERR;

	/* check if sensor is initialized */
	res = bme280_is_sleep_mode(Dev);
	if(BME280_NO_INIT_ERR == res) return res;

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
	uint8_t config = 0, tmp = 0;

	/* check parameters */
	if( IS_NULL(Dev) || (Filter > BME280_FILTER_16) ) return BME280_PARAM_ERR;

	/* check if sensor is initialized and in sleep mode */
	res = bme280_is_sleep_mode(Dev);
	if(BME280_OK != res) return res;

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
	uint8_t config = 0, tmp = 0;

	/* check parameter */
	if( IS_NULL(Dev) ) return BME280_PARAM_ERR;

	/* check if sensor is initialized and in sleep mode */
	res = bme280_is_sleep_mode(Dev);
	if(BME280_OK != res) return res;

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
	uint8_t config = 0, tmp = 0;

	/* check parameter */
	if( IS_NULL(Dev) ) return BME280_PARAM_ERR;

	/* check if sensor is initialized and in sleep mode */
	res = bme280_is_sleep_mode(Dev);
	if(BME280_OK != res) return res;

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
	if( IS_NULL(Dev) || IS_NULL(Result) ) return BME280_PARAM_ERR;

	/* check if sensor is initialized */
	res = bme280_is_sleep_mode(Dev);
	if(BME280_NO_INIT_ERR == res) return res;

	/* read value of ctrl_meas register from sensor */
	res = Dev->read(BME280_CONFIG_ADDR, &config, 1, Dev->i2c_address, Dev->env_spec_data);
	if(BME280_OK != res) return BME280_INTERFACE_ERR;

	/* parse mode values from ctrl_meas */
	config &= 0x01;

	/* set output pointer */
	*Result = config;

	return res;
}

	/* function reads last measured values from sensor in normal mode (no floats) */
int8_t BME280_ReadLastAll(BME280_t *Dev, BME280_Data_t *Data){

	int8_t res = BME280_OK;
	BME280_S32_t temp;
	BME280_U32_t press, hum;

	/* check parameters */
	if( IS_NULL(Dev) || IS_NULL(Data) ) return BME280_PARAM_ERR;

	/* check if sensor is initialized and in normal mode */
	res = bme280_is_normal_mode(Dev);
	if(BME280_OK != res) return res;

	/* read the data from sensor */
	res = bme280_read_compensate(read_all, Dev, &temp, &press, &hum);
	if(BME280_OK != res) return res;

	/* convert 32bit values to Data structure */
	bme280_convert_t_S32_struct(temp, Data);
	bme280_convert_p_U32_struct(press, Data);
	bme280_convert_h_U32_struct(hum, Data);

	return res;
}

	/* function reads last measured values from sensor in normal mode (with floats) */
int8_t BME280_ReadLastAll_F(BME280_t *Dev, BME280_DataF_t *Data){

	int8_t res = BME280_OK;
	BME280_S32_t temp;
	BME280_U32_t press, hum;

	/* check parameters */
	if( IS_NULL(Dev) || IS_NULL(Data) ) return BME280_PARAM_ERR;

	/* check if sensor is initialized and in normal mode */
	res = bme280_is_normal_mode(Dev);
	if(BME280_OK != res) return res;

	/* read the data from sensor */
	res = bme280_read_compensate(read_all, Dev, &temp, &press, &hum);
	if(BME280_OK != res) return res;

	/* convert 32bit values to Data structure */
	bme280_convert_t_S32_float(temp, &Data->temp);
	bme280_convert_p_U32_float(press, &Data->press);
	bme280_convert_h_U32_float(hum, &Data->hum);

	return res;
}

	/* function reads last measured temperature from sensor in normal mode (no floats) */
int8_t BME280_ReadLastTemp(BME280_t *Dev, int8_t *TempInt, uint8_t *TempFract){

	int8_t res = BME280_OK;
	BME280_S32_t temp;
	BME280_Data_t data;

	/* check parameters */
	if( IS_NULL(Dev) || IS_NULL(TempInt) || IS_NULL(TempFract) ) return BME280_PARAM_ERR;

	/* check if sensor is initialized and in normal mode */
	res = bme280_is_normal_mode(Dev);
	if(BME280_OK != res) return res;

	/* read the data from sensor */
	res = bme280_read_compensate(read_temp, Dev, &temp, 0, 0);
	if(BME280_OK != res) return res;

	/* convert 32bit values to local data structure */
	bme280_convert_t_S32_struct(temp, &data);

	/* set values of external variables */
	*TempInt = data.temp_int;
	*TempFract = data.temp_fract;

	return res;
}

	/* function reads last measured temperature from sensor in normal mode (with floats) */
int8_t BME280_ReadLastTemp_F(BME280_t *Dev, float *Temp){

	int8_t res = BME280_OK;
	BME280_S32_t temp;

	/* check parameters */
	if( IS_NULL(Dev) || IS_NULL(Temp) ) return BME280_PARAM_ERR;

	/* check if sensor is initialized and in normal mode */
	res = bme280_is_normal_mode(Dev);
	if(BME280_OK != res) return res;

	/* read the data from sensor */
	res = bme280_read_compensate(read_temp, Dev, &temp, 0, 0);
	if(BME280_OK != res) return res;

	/* convert 32bit value to external float */
	bme280_convert_t_S32_float(temp, Temp);

	return res;
}

	/* function reads last measured pressure from sensor in normal mode (no floats) */
int8_t BME280_ReadLastPress(BME280_t *Dev, uint16_t *PressInt, uint16_t *PressFract){

	int8_t res = BME280_OK;
	BME280_S32_t temp;
	BME280_U32_t press;
	BME280_Data_t data;

	/* check parameters */
	if( IS_NULL(Dev) || IS_NULL(PressInt) || IS_NULL(PressFract) ) return BME280_PARAM_ERR;

	/* check if sensor is initialized and in normal mode */
	res = bme280_is_normal_mode(Dev);
	if(BME280_OK != res) return res;

	/* read the data from sensor */
	res = bme280_read_compensate(read_press, Dev, &temp, &press, 0);
	if(BME280_OK != res) return res;

	/* convert 32bit value to local data structure */
	bme280_convert_p_U32_struct(press, &data);

	/* set values of external variables */
	*PressInt = data.pressure_int;
	*PressFract = data.pressure_fract;

	return res;
}

	/* function reads last measured pressure from sensor in normal mode (with floats) */
int8_t BME280_ReadLastPress_F(BME280_t *Dev, float *Press){

	int8_t res = BME280_OK;
	BME280_S32_t temp;
	BME280_U32_t press;

	/* check parameters */
	if( IS_NULL(Dev) || IS_NULL(Press) ) return BME280_PARAM_ERR;

	/* check if sensor is initialized and in normal mode */
	res = bme280_is_normal_mode(Dev);
	if(BME280_OK != res) return res;

	/* read the data from sensor */
	res = bme280_read_compensate(read_press, Dev, &temp, &press, 0);
	if(BME280_OK != res) return res;

	/* convert 32bit value to external float */
	bme280_convert_p_U32_float(press, Press);

	return res;
}

	/* function reads last measured humidity from sensor in normal mode (no floats) */
int8_t BME280_ReadLastHum(BME280_t *Dev, uint8_t *HumInt, uint16_t *HumFract){

	int8_t res = BME280_OK;
	BME280_S32_t temp;
	BME280_U32_t hum;
	BME280_Data_t data;

	/* check parameters */
	if( IS_NULL(Dev) || IS_NULL(HumInt) || IS_NULL(HumFract) ) return BME280_PARAM_ERR;

	/* check if sensor is initialized and in normal mode */
	res = bme280_is_normal_mode(Dev);
	if(BME280_OK != res) return res;

	/* read the data from sensor */
	res = bme280_read_compensate(read_hum, Dev, &temp, 0, &hum);
	if(BME280_OK != res) return res;

	/* convert 32bit value to local data structure */
	bme280_convert_h_U32_struct(hum, &data);

	/* set values of external variables */
	*HumInt = data.humidity_int;
	*HumFract = data.humidity_fract;

	return res;
}

	/* function reads last measured humidity from sensor in normal mode (with floats) */
int8_t BME280_ReadLastHum_F(BME280_t *Dev, float *Hum){

	int8_t res = BME280_OK;
	BME280_S32_t temp;
	BME280_U32_t hum;

	/* check parameters */
	if( IS_NULL(Dev) || IS_NULL(Hum) ) return BME280_PARAM_ERR;

	/* check if sensor is initialized and in normal mode */
	res = bme280_is_normal_mode(Dev);
	if(BME280_OK != res) return res;

	/* read the data from sensor */
	res = bme280_read_compensate(read_hum, Dev, &temp, 0, &hum);
	if(BME280_OK != res) return res;

	/* convert 32bit value to external float */
	bme280_convert_h_U32_float(hum, Hum);

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

	/* private function to read and compensate selected adc
	 * data from sensor  */
static int8_t bme280_read_compensate(uint8_t read_type, BME280_t *Dev, BME280_S32_t *temp,
		BME280_U32_t *press, BME280_U32_t *hum){

	int8_t res = BME280_OK;
	BME280_S32_t adc_T, adc_P, adc_H;
	struct adc_regs adc_raw;

	/* read selected adc data from sensor */
	switch(read_type){

	case read_temp:
		res = Dev->read(BME280_TEMP_ADC_ADDR, (uint8_t *)&adc_raw.temp_raw, BME280_TEMP_ADC_LEN,
				Dev->i2c_address, Dev->env_spec_data);
		break;

	case read_press:
		res = Dev->read(BME280_PRESS_ADC_ADDR, (uint8_t *)&adc_raw.press_raw, (BME280_PRESS_ADC_LEN +
				BME280_TEMP_ADC_LEN), Dev->i2c_address, Dev->env_spec_data);
		break;

	case read_hum:
		res = Dev->read(BME280_TEMP_ADC_ADDR, (uint8_t *)&adc_raw.temp_raw, (BME280_TEMP_ADC_LEN +
				BME280_HUM_ADC_LEN), Dev->i2c_address, Dev->env_spec_data);
		break;

	case read_all:
		res = Dev->read(BME280_PRESS_ADC_ADDR, (uint8_t *)&adc_raw.press_raw, (BME280_PRESS_ADC_LEN +
				BME280_TEMP_ADC_LEN + BME280_HUM_ADC_LEN), Dev->i2c_address, Dev->env_spec_data);
		break;

	default:
		return BME280_PARAM_ERR;
		break;
	}

	/* parse  and compensate data from adc_raw structure to variables */
	adc_T = bme280_parse_press_temp_s32t((uint8_t *)&adc_raw.temp_raw);
	*temp = bme280_compensate_t_s32t(Dev, adc_T);

	if((read_press == read_type) || (read_all == read_type)){

		adc_P = bme280_parse_press_temp_s32t((uint8_t *)&adc_raw.press_raw);
		*press = bme280_compensate_p_u32t(Dev, adc_P);
	}

	if((read_hum == read_type) || (read_all == read_type)){

		adc_H = bme280_parse_hum_s32t((uint8_t *)&adc_raw.hum_raw);
		*hum = bme280_compensate_h_u32t(Dev, adc_H);
	}

	return res;
}

	/* private function that parses raw adc pressure or temp values
	 * from sensor into a single BME280_S32_t variable */
static BME280_S32_t bme280_parse_press_temp_s32t(uint8_t *raw){

	BME280_S32_t res;

	res = ((BME280_S32_t)raw[0] << 12U) | ((BME280_S32_t)raw[1] << 4U) | ((BME280_S32_t)raw[2] >> 4U);
	res &= 0xFFFFF;

	return res;
}

	/* private function that parses raw adc humidity values
	 * from sensor into a single BME280_S32_t variable */
static BME280_S32_t bme280_parse_hum_s32t(uint8_t *raw){

	BME280_S32_t res;

	res = ((BME280_S32_t)raw[0] << 8U) | ((BME280_S32_t)raw[1]);
	res &= 0xFFFF;

	return res;
}

	/* Returns temperature in DegC, resolution is 0.01 DegC. Output value of “5123”
	 * equals 51.23 DegC. t_fine carries fine temperature as global value */
static BME280_S32_t bme280_compensate_t_s32t(BME280_t *Dev, BME280_S32_t adc_T){

	BME280_S32_t var1;
	BME280_S32_t var2;
	BME280_S32_t temperature;

    var1 = (BME280_S32_t)((adc_T / 8) - ((BME280_S32_t)Dev->trimm.dig_T1 * 2));
    var1 = (var1 * ((BME280_S32_t)Dev->trimm.dig_T2)) / 2048;
    var2 = (BME280_S32_t)((adc_T / 16) - ((BME280_S32_t)Dev->trimm.dig_T1));
    var2 = (((var2 * var2) / 4096) * ((BME280_S32_t)Dev->trimm.dig_T3)) / 16384;
    Dev->t_fine = var1 + var2;
    temperature = (Dev->t_fine * 5 + 128) / 256;

    return temperature;
}

	/* Returns pressure in Pa as unsigned 32 bit integer in Q24.8 format
	 * (24 integer bits and 8 fractional bits). Output value of “24674867”
	 * represents 24674867/256 = 96386.2 Pa = 963.862 hPa */
#ifdef USE_64BIT
static BME280_U32_t bme280_compensate_p_u32t(BME280_t *Dev, BME280_S32_t adc_P){

	BME280_S64_t var1;
	BME280_S64_t var2;
	BME280_S64_t var3;
	BME280_S64_t var4;
	BME280_U32_t pressure;

    var1 = ((BME280_S64_t)Dev->t_fine) - 128000;
    var2 = var1 * var1 * (BME280_S64_t)Dev->trimm.dig_P6;
    var2 = var2 + ((var1 * (BME280_S64_t)Dev->trimm.dig_P5) * 131072);
    var2 = var2 + (((BME280_S64_t)Dev->trimm.dig_P4) * 34359738368);
    var1 = ((var1 * var1 * (BME280_S64_t)Dev->trimm.dig_P3) / 256) + ((var1 * ((BME280_S64_t)Dev->trimm.dig_P2) * 4096));
    var3 = ((BME280_S64_t)1) * 140737488355328;
    var1 = (var3 + var1) * ((BME280_S64_t)Dev->trimm.dig_P1) / 8589934592;

    /* To avoid divide by zero exception */
    if (var1 != 0)
    {
        var4 = 1048576 - adc_P;
        var4 = (((var4 * INT64_C(2147483648)) - var2) * 3125) / var1;
        var1 = (((BME280_S64_t)Dev->trimm.dig_P9) * (var4 / 8192) * (var4 / 8192)) / 33554432;
        var2 = (((BME280_S64_t)Dev->trimm.dig_P8) * var4) / 524288;
        var4 = ((var4 + var1 + var2) / 256) + (((BME280_S64_t)Dev->trimm.dig_P7) * 16);
        pressure = (BME280_U32_t)(((var4 / 2) * 100) / 128);

    }
    else
    {
        pressure = 0;
    }

    return pressure;
}
#else
static BME280_U32_t bme280_compensate_p_u32t(BME280_t *Dev, BME280_S32_t adc_P){

	BME280_S32_t var1, var2;
	BME280_U32_t pressure;

	var1 = (((BME280_S32_t)Dev->t_fine)>>1) - (BME280_S32_t)64000;
	var2 = (((var1>>2) * (var1>>2)) >> 11 ) * ((BME280_S32_t)Dev->trimm.dig_P6);
	var2 = var2 + ((var1*((BME280_S32_t)Dev->trimm.dig_P5))<<1);
	var2 = (var2>>2)+(((BME280_S32_t)Dev->trimm.dig_P4)<<16);
	var1 = (((Dev->trimm.dig_P3 * (((var1>>2) * (var1>>2)) >> 13 )) >> 3) + ((((BME280_S32_t)Dev->trimm.dig_P2) *
	var1)>>1))>>18;
	var1 =((((32768+var1))*((BME280_S32_t)Dev->trimm.dig_P1))>>15);
	if (var1 == 0)
	{
	return 0; // avoid exception caused by division by zero
	}
	pressure = (((BME280_U32_t)(((BME280_S32_t)1048576)-adc_P)-(var2>>12)))*3125;
	if (pressure < 0x80000000)
	{
	pressure = (pressure << 1) / ((BME280_U32_t)var1);
	}
	else
	{
	pressure = (pressure / (BME280_U32_t)var1) * 2;
	}
	var1 = (((BME280_S32_t)Dev->trimm.dig_P9) * ((BME280_S32_t)(((pressure>>3) * (pressure>>3))>>13)))>>12;
	var2 = (((BME280_S32_t)(pressure>>2)) * ((BME280_S32_t)Dev->trimm.dig_P8))>>13;
	pressure = (BME280_U32_t)((BME280_S32_t)pressure + ((var1 + var2 + Dev->trimm.dig_P7) >> 4));
	return pressure;
}
#endif

	/* Returns humidity in %RH as unsigned 32bit integer in Q22.10 format (22 integer
	 * and 10 fractional bits). Output value of "47445" represents 47445/1024 = 46.333 %RH */
static BME280_U32_t bme280_compensate_h_u32t(BME280_t *Dev, BME280_S32_t adc_H){

	BME280_S32_t var1;
	BME280_S32_t var2;
	BME280_S32_t var3;
	BME280_S32_t var4;
	BME280_S32_t var5;
	BME280_U32_t humidity;

    var1 = Dev->t_fine - ((BME280_S32_t)76800);
    var2 = (BME280_S32_t)(adc_H * 16384);
    var3 = (BME280_S32_t)(((BME280_S32_t)Dev->trimm.dig_H4) * 1048576);
    var4 = ((BME280_S32_t)Dev->trimm.dig_H5) * var1;
    var5 = (((var2 - var3) - var4) + (BME280_S32_t)16384) / 32768;
    var2 = (var1 * ((BME280_S32_t)Dev->trimm.dig_H6)) / 1024;
    var3 = (var1 * ((BME280_S32_t)Dev->trimm.dig_H3)) / 2048;
    var4 = ((var2 * (var3 + (BME280_S32_t)32768)) / 1024) + (BME280_S32_t)2097152;
    var2 = ((var4 * ((BME280_S32_t)Dev->trimm.dig_H2)) + 8192) / 16384;
    var3 = var5 * var2;
    var4 = ((var3 / 32768) * (var3 / 32768)) / 128;
    var5 = var3 - ((var4 * ((BME280_S32_t)Dev->trimm.dig_H1)) / 16);
    var5 = (var5 < 0 ? 0 : var5);
    var5 = (var5 > 419430400 ? 419430400 : var5);
    humidity = (BME280_U32_t)(var5 / 4096);

    return humidity;
}

	/* function converts BME280_S32_t temperature to BME280_Data_t structure */
static void bme280_convert_t_S32_struct(BME280_S32_t temp, BME280_Data_t *data){

	data->temp_int = (BME280_S32_t)temp / 100;
	data->temp_fract = (BME280_S32_t)temp % 100;
}

	/* function converts BME280_S32_t temperature to float */
static void bme280_convert_t_S32_float(BME280_S32_t temp_in, float *temp_out){

	*temp_out= (float)temp_in / 100.0F;
}

	/* function converts BME280_U32_t pressure to BME280_Data_t structure */
#ifdef USE_64BIT
static void bme280_convert_p_U32_struct(BME280_U32_t press, BME280_Data_t *data){

	data->pressure_int = press / (BME280_U32_t)10000;
	data->pressure_fract = (press % (BME280_U32_t)10000) / (BME280_U32_t)10;
}
#else
static void bme280_convert_p_U32_struct(BME280_U32_t press, BME280_Data_t *data){

	data->pressure_int = press / (BME280_U32_t)100;
	data->pressure_fract = press % (BME280_U32_t)100;
}
#endif

	/* function converts BME280_U32_t pressure to float */
#ifdef USE_64BIT
static void bme280_convert_p_U32_float(BME280_U32_t press_in, float *press_out){

	*press_out = (float)press_in / 10000.0F;
}
#else
static void bme280_convert_p_U32_float(BME280_U32_t press_in, float *press_out){{

	*press_out  = (float)press_in / 100.0F;
}
#endif

	/* function converts BME280_U32_t humidity to BME280_Data_t structure */
static void bme280_convert_h_U32_struct(BME280_U32_t hum, BME280_Data_t *data){

	data->humidity_int = hum / (BME280_U32_t)1000;
	data->humidity_fract = hum % (BME280_U32_t)1000;
}

	/* function converts BME280_U32_t humidity to float */
static void bme280_convert_h_U32_float(BME280_U32_t hum_in, float *hum_out){

	*hum_out = (float)hum_in / 1000.0F;
}

	/* function checks if device was initialized and is in normal mode */
static int8_t bme280_is_normal_mode(BME280_t *Dev){

	if(BME280_NOT_INITIALIZED == Dev->initialized) {return BME280_NO_INIT_ERR;}

	if(normal_mode != Dev->mode) return BME280_CONDITION_ERR;

	return BME280_OK;
}

	/* function checks if device was initialized and is in sleep mode */
static int8_t bme280_is_sleep_mode(BME280_t *Dev){

	if(BME280_NOT_INITIALIZED == Dev->initialized) {return BME280_NO_INIT_ERR;}

	if(sleep_mode != Dev->mode) return BME280_CONDITION_ERR;

	return BME280_OK;
}
