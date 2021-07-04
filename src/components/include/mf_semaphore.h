#ifndef _MF_SEMAPHORE_H_
#define _MF_SEMAPHORE_H_
#include <stdint.h>
#include "oc_api.h"

uint8_t mf_semaphore_create_resource(oc_resource_t *semaph);
uint8_t mf_semaphore_init(void);
uint8_t mf_semaphore_destroy(void);
#endif