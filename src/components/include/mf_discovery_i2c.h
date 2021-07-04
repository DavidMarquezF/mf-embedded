#ifndef _MF_DISCOVERY_I2C_H_
#define _MF_DISCOVERY_I2C_H_
#include <stdint.h>
#include <stdbool.h>
#include "mf_device.h"


uint8_t mf_discovery_i2c_init_discovery(void);

/**
 * @brief: Gets the next available i2c device
 * 
 * @param: device The device to be set, if it can't find any more devices it will be set to MF_DEVICE_INVALID 
 * 
 * @return: If there has been an error trying to discover the device
 */
uint8_t mf_discovery_i2c_discover_next(mf_device_t *device);


#endif
