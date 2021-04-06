#ifndef _MF_SPI_H_
#define _MF_SPI_H_
#include <stdint.h>
#include <stddef.h>

#ifndef MF_SPI_MAX_DEVICES
#define MF_SPI_MAX_DEVICES 4
#endif

typedef int mf_spi_device_t;
#define MF_INVALID_SPI_DEVICE -1

uint8_t mf_spi_init(void);
mf_spi_device_t mf_spi_add_device(uint8_t enablepin);
uint8_t mf_spi_send_message(mf_spi_device_t device, uint8_t cmd, uint8_t* message, size_t length);
uint8_t mf_spi_receive_message(mf_spi_device_t device, void* receive_buffer, size_t length);

#endif