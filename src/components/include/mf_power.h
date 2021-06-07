#ifndef _MF_POWER_H_
#define _MF_POWER_H_

#include <stdbool.h>
#include <stdint.h>

uint8_t mf_power_init(void);
void mf_power_enable_modules(bool enable);


#endif