#ifndef _MF_ULTRASOUND_H_
#define _MF_ULTRASOUND_H_
#include <stdint.h>
#include "oc_api.h"

uint8_t mf_ultrasound_create_resource(oc_resource_t *ultrasound_resource);
uint8_t mf_ultrasound_init(void);
uint8_t mf_ultrasound_destroy(void);

#endif