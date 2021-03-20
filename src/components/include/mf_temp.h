#ifndef _MF_TEMP_H_
#define _MF_TEMP_H_
#include <stdint.h>

uint8_t mf_temp_setup(void);
uint8_t mf_temp_get_value(float* temp_value);
uint8_t mf_temp_destroy(void);
#endif