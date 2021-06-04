#ifndef _MF_I2C_H_
#define _MF_I2C_H_
#include <stdint.h>
#include <stddef.h>
#include "mf_device.h"


typedef int mf_i2c_device_t;
#define MF_I2C_INVALID_DEVICE -1

/**
 * @brief: Sets up i2c with all the devices
 * 
 */ 
uint8_t mf_i2c_init(void);
uint8_t mf_i2c_send_message(mf_i2c_device_t device, uint8_t* message, size_t length);
uint8_t mf_i2c_receive_message(mf_i2c_device_t device, void* receive_buffer, size_t length);
uint8_t mf_i2c_send_and_receive_message(mf_i2c_device_t device, uint8_t *message, size_t message_size, void* receive_buffer, size_t receive_size);

#endif