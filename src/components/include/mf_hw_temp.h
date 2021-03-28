#ifndef _MF_HW_TEMP_H_
#define _MF_HW_TEMP_H_
#include <stdint.h>

uint8_t mf_hw_temp_init(void);
uint8_t mf_hw_temp_get_value(float* temp_value);
uint8_t mf_hw_temp_destroy(void);
#endif