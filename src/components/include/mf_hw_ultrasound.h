#ifndef _MF_HW_ULTRASOUND_H_
#define _MF_HW_ULTRASOUND_H_
#include <stdint.h>

uint8_t mf_hw_ultrasound_init(void);
uint8_t mf_hw_ultrasound_get_value(float* temp_value);
uint8_t mf_hw_ultrasound_destroy(void);
#endif