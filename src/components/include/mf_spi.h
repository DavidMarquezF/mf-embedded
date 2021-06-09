#ifndef _MF_SPI_H_
#define _MF_SPI_H_
#include <stdint.h>
#include <stddef.h>
#include "mf_device.h"

#ifndef MF_SPI_MAX_DEVICES
#define MF_SPI_MAX_DEVICES 3
#endif

typedef int mf_spi_device_t;
#define MF_SPI_INVALID_DEVICE -1

/**
 * @brief: Sets up spi with all the devices
 * 
 * @param: enablePins: Specify the CS pin number. Must be of length MF_SPI_MAX_DEVICES
 */ 
uint8_t mf_spi_init(uint8_t * enablePins);
/**
 * @brief: Returns the spi device from an index 
 * 
 * @param: index Must be constrained between 0 and MF_SPI_MAX_DEVICES 
 */
mf_spi_device_t mf_spi_device_from_index(uint8_t index);

uint8_t mf_spi_send_message(mf_spi_device_t device, uint8_t cmd, uint8_t* message, size_t length);
uint8_t mf_spi_receive_message(mf_spi_device_t device, void* receive_buffer, size_t length);
uint8_t mf_spi_send_and_receive_message(mf_spi_device_t device,uint8_t cmd, uint8_t *message, size_t message_size, void* receive_buffer, size_t receive_size);

#endif