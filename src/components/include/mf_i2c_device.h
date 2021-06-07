#ifndef _MF_I2C_DEVICE_H_
#define _MF_I2C_DEVICE_H_
#include <stdint.h>
#include <stddef.h>
#include "mf_device.h"
#include "mf_i2c.h"

#ifndef MF_I2C_MAX_DEVICES
#define MF_I2C_MAX_DEVICES 1
#endif
uint8_t mf_i2c_device_discover_devices(void);
uint8_t mf_i2c_device_get_value(mf_device_t dev, void * out_result, size_t size);
uint8_t mf_i2c_device_set_value(mf_device_t dev, void * value, size_t size);
mf_i2c_device_t mf_i2c_device_get_device(mf_device_t dev);
#endif