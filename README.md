BME280 Driver
=============
![version](https://img.shields.io/github/v/release/JuraszekL/BME280_Driver?color=brightgreen)

![BME280](https://raw.githubusercontent.com/JuraszekL/BME280_Driver/master/Resources/BME280.jpeg)
![Bosch](https://raw.githubusercontent.com/JuraszekL/BME280_Driver/master/Resources/Bosch.png)

### For Doxygen documentation click [here](https://juraszekl.github.io/BME280_Driver/index.html). (only latest version)
Offline doxygen documentation for current version is attached as Doc.zip file. Unzip this file and open index.html in your browser.

### Driver for Bosch BME280 Combined sensor.
[BME280 Datasheet](https://www.bosch-sensortec.com/media/boschsensortec/downloads/datasheets/bst-bme280-ds002.pdf)

### Features

- Communication with I2C bus only
- Read all measured values in Normal and Forced mode
- Results returned as integers or floats
- Configurable use 32-bit variables only (when 64-bit are not avalible)
- No dynamic memory allocation used

Driver is still under development, next features will be add soon.
Current version - v1.1.x

### Sensor description

The BME280 is a humidity sensor especially developed for mobile applications and wearables where size and low power consumption are key design parameters.
The unit combines high linearity and high accuracy sensors and is perfectly feasible for low current consumption, long-term stability and high EMC robustness.
The humidity sensor offers an extremely fast response time and therefore supports performance requirements for emerging applications such as context awareness,
and high accuracy over a wide temperature range.

Motivation
----------

This project is for learning purposes only. Feel free to use it.

How to use
----------

To use this driver perform steps listed below:

### 1. Clone driver into your workign directory:
```console
git clone https://github.com/JuraszekL/BME280_Driver.git
```
or use as submodule if your working directory is a git repo
```console
git submodule add https://github.com/JuraszekL/BME280_Driver.git
```

### 2. Include **__bme280.h__** file:
```c
#include "bme280.h"
```

### 3. Perform configuration in **__bme280.h__** file if needed:
```c
/// comment this line if you don't want to use 64bit variables in calculations
#define USE_64BIT
/// comment this line if you don't need to use functions with floating point results
#define USE_FLOATS_RESULTS
/// comment this line if you don't need to use functions with integer results
#define USE_INTEGER_RESULTS
/// comment this line if you don't need to read single setting with any getX function
#define USE_GETTERS
/// comment this line if you don't need to write single setting with any setX function
#define USE_SETTERS
/// comment this line if you don't use functionns to read data in normal mode (BME280_ReadxxxLast/BME280_ReadxxxLast_F)
#define USE_NORMAL_MODE
/// comment this line if you don't use functionns to read data in forced mode (BME280_ReadxxxForce/BME280_ReadxxxForce_F)
#define USE_FORCED_MODE
```

### 4. Write platform-specific functions required by driver:

- Read Function:
```c
/**
 * Function to read the data from sensor in burst mode.
 * @param[in] reg_addr address of register to be read (f.e. #BME280_ID_ADDR)
 * @param[in] *rxbuff pointer to the buffer where data will be stored
 * @param[in] rxlen lenght of data to be read (in bytes)
 * @param[in] dev_addr address of the device on I2C bus (#BME280_I2CADDR_SDOL or #BME280_I2CADDR_SDOH)
 * @param[in] env_spec_data pointer to platform specific data required to perform bus operation
 * (f.e. pointer to i2c bus strucure)
 * @return 0 success
 * @return -1 failure
 */
typedef int8_t (*bme280_readbytes)(uint8_t reg_addr, uint8_t *rxbuff, uint8_t rxlen, uint8_t dev_addr, void *env_spec_data);
```

- Write Function:
```c
/**
 * Function to write one byte to sensor.
 * @param[in] reg_addr address of register to be written (f.e. #BME280_RESET_ADDR)
 * @param[in] value value to write (f.e. #BME280_RESET_VALUE)
 * @param[in] dev_addr address of the device on I2C bus (#BME280_I2CADDR_SDOL or #BME280_I2CADDR_SDOH)
 * @param[in] env_spec_data pointer to platform specific data required to perform bus operation
 * (f.e. pointer to i2c bus strucure)
 * @return 0 success
 * @return -1 failure
 */
typedef int8_t (*bme280_writebyte)(uint8_t reg_addr, uint8_t value, uint8_t dev_addr, void *env_spec_data);
```

- Delay Function:
```c
/**
 * Delay function.
 * @param[in] delay_time time to delay in miliseconds
 */
typedef void (*bme280_delayms)(uint8_t delay_time);
```
### 5. Create global BME280_Driver_t structure and fill it with platform specific data:

```c
typedef struct {

	/// current address on I2C bus, value should be #BME280_I2CADDR_SDOL or #BME280_I2CADDR_SDOH
	uint8_t i2c_address;
	/// pointer to platform specific data (f.e. to i2c bus structure)
	void *env_spec_data;
	/// pointer to user defined function that reads data from sensor
	bme280_readbytes read;
	/// pointer to user defined function that writes data to sensor
	bme280_writebyte write;
	/// pointer to user defined delay function
	bme280_delayms delay;

} BME280_Driver_t;
```
### 6. Use BME280_Init Function before any operation:

BME280_t *Dev structure is a reference for single sensor you want to work with. Should be global as well.
Use pointer to this structure with any other functions from this driver.

```c
/**
 * @brief Function to initialize sensor and resources
 * @note This must be a first function usend in code before any other opearion can be performed!
 *
 * Init funtion performs sensor reset and checks #BME280_ID. It doesn't set any sensor's parameters. Calibration
 * data specific for each one sensor are read while Init function. If operation is completed with
 * success function sets "initialized" value in #BME280_t structure.
 * @param[in] *Dev pointer to #BME280_t structure which should be initialized
 * @param[in] *Driver pointer to BME280_Driver_t structure where all platform specific data are stored. This structure
 * MUST exist while program is running - do not use local structures to init sensor!
 * @return #BME280_OK success
 * @return #BME280_PARAM_ERR wrong parameter passed
 * @return #BME280_INTERFACE_ERR user defined read/write function returned non-zero value
 * @return #BME280_ID_ERR sensor's id doesnt match with #BME280_ID
 */
int8_t BME280_Init(BME280_t *Dev, BME280_Driver_t *Driver);
```
Optional steps
--------------

- Change defined types if needed:
```c
typedef int32_t BME280_S32_t;	///< signed 32-bit integer variable
typedef uint32_t BME280_U32_t;	///< unsigned 32-bit integer variable
typedef int64_t BME280_S64_t;	///< signed 64-bit integer variable
```
- Set all sensor's settings at once (after Init function)
```c
/**
 * @brief Function to set all sensor settings at once
 * @note Sensor must be in #BME280_SLEEPMODE to set all parameters. Force #BME280_SLEEPMODE with #BME280_SetMode
 * function before or use it directly after #BME280_Init function only.
 *
 * Function writes all 3 config registers without reading them before. It can be usefull after power-up or reset.
 * It sets current operating mode inside *Dev structure at the end.
 * @param[in] *Dev pointer to sensor's #BME280_t structure
 * @param[in] *Config pointer to #BME280_Config_t structure which contains all paramaters to be set
 * @return #BME280_OK success
 * @return #BME280_PARAM_ERR wrong parameter passed
 * @return #BME280_INTERFACE_ERR user defined read/write function returned non-zero value
 * @return #BME280_NO_INIT_ERR sensor was not initialized before
 * @return #BME280_CONDITION_ERR sensor is not in #BME280_SLEEPMODE
 */
int8_t BME280_ConfigureAll(BME280_t *Dev, BME280_Config_t *Config);
```