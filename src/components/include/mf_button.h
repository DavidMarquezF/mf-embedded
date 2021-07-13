#ifndef _MF_BUTTON_H_
#define _MF_BUTTON_H_
#include <stdint.h>
#include "oc_api.h"

uint8_t mf_button_create_resource(oc_resource_t *semaph);
uint8_t mf_button_init(void);
uint8_t mf_button_destroy(void);
#endif