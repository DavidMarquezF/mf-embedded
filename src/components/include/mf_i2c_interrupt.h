#ifndef _MF_I2C_INTERRUPT_H_
#define _MF_I2C_INTERRUPT_H_
#include <stdint.h>
#include "mf_device.h"
typedef void (*mf_component_notify_t)(void);

uint8_t mf_i2c_interrupt_init(uint8_t* pins, uint8_t len);
uint8_t mf_i2c_interrupt_register(mf_device_t dev, mf_component_notify_t cb);

#endif
