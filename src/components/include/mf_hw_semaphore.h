#ifndef _MF_SEMAPHORE_HW_H_
#define _MF_SEMAPHORE_HW_H_
#include <stdint.h>
#include <stdbool.h>

uint8_t mf_hw_semaphore_init(void);
uint8_t mf_hw_semaphore_set_value(bool green, bool yellow, bool red);
uint8_t mf_hw_semaphore_destroy(void);
#endif