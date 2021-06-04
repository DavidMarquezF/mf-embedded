#ifndef _MF_SEMAPHORE_H_
#define _MF_SEMAPHORE_H_
#include <stdint.h>
#include <stdbool.h>

uint8_t mf_hw_semaphore_init(void);
uint8_t mf_hw_semaphore_set_value(bool s1, bool s2, bool s3);
uint8_t mf_hw_semaphore_destroy(void);
#endif