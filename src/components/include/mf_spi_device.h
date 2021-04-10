#ifndef _MF_SPI_DEVICE_H_
#define _MF_SPI_DEVICE_H_
#include <stdint.h>
#include <stddef.h>
#include "mf_device.h"
#include "mf_spi.h"

uint8_t mf_spi_device_discover_devices(void);
uint8_t mf_spi_device_get_value(mf_device_t dev, void * out_result, size_t size);
mf_spi_device_t mf_spi_device_get_device(mf_device_t dev);
mf_device_t mf_spi_device_get_device_from_index(uint8_t index);
#endif