#ifndef _MF_TEMP_H_
#define _MF_TEMP_H_
#include <stdint.h>
#include "oc_api.h"

uint8_t mf_temp_create_resource(oc_resource_t *temp_resource);
uint8_t mf_temp_init(void);
uint8_t mf_temp_destroy(void);
#endif