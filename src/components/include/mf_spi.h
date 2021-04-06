#ifndef _MF_SPI_H_
#define _MF_SPI_H_
#include <stdint.h>
#include <stddef.h>

uint8_t mf_spi_init(void);
uint8_t mf_spi_send_message(uint8_t cmd, uint8_t* message, size_t length);
uint8_t mf_spi_receive_message(void* receive_buffer, size_t length);

#endif