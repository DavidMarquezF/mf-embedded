#ifndef _MF_COMPONENT_HANDLER_H_
#define _MF_COMPONENT_HANDLER_H_
#include <stdint.h>
#include "oc_api.h"

#define MF_COMPONENT_MAX_URL_LEN 20
#define MF_COMPONENT_MAX_NAME_LEN 20


typedef uint8_t (*mf_component_create_resource_t)(oc_resource_t *temp_resource);
typedef uint8_t (*mf_component_init_t)(void);
typedef uint8_t (*mf_component_destroy_t)(void);


typedef struct {
    char url [MF_COMPONENT_MAX_URL_LEN];
    char name [MF_COMPONENT_MAX_NAME_LEN];
    mf_component_create_resource_t create_resource_callback;
    mf_component_init_t init_callback;
    mf_component_destroy_t destroy_callback;

} mf_component_config_t;


uint8_t mf_component_handler_init(void);

uint8_t mf_component_handler_register(mf_component_config_t config);

uint8_t mf_component_handler_init_components(void);
uint8_t mf_component_handler_destroy_components(void);

#endif