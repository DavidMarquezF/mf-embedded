#ifndef _MF_SWITCH_ACTUATOR_H_
#define _MF_SWITCH_ACTUATOR_H_
#include <stdint.h>
#include "oc_api.h"

uint8_t mf_switch_actuator_create_resource(oc_resource_t *temp_resource);
uint8_t mf_switch_actuator_init(void);
uint8_t mf_switch_actuator_destroy(void);
#endif