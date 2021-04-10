#ifndef _MF_GEN_COMPONENTS_H_
#define _MF_GEN_COMPONENTS_H_

#include "../mf_component_handler.h"
#include "../mf_temp.h"
#include "../mf_ultrasound.h"
#include "../mf_switch_actuator.h"

// MF_NUM_COMPONENTS has to be set when compiling in order to optimize memory usage. The number of components will be known prior to compilation

 mf_component_config_t generated_components[] = {{.name = "TempSetter",
                                     .url = "/temp",
                                     .create_resource_callback = mf_temp_create_resource,
                                     .init_callback = mf_temp_init,
                                     .destroy_callback = mf_temp_destroy},
                                    {.name = "Dist",
                                     .url = "/distance",
                                     .create_resource_callback = mf_ultrasound_create_resource,
                                     .init_callback = mf_ultrasound_init,
                                     .destroy_callback = mf_ultrasound_destroy},

                                    {.name = "Led",
                                     .url = "/led",
                                     .create_resource_callback = mf_switch_actuator_create_resource,
                                     .init_callback = mf_switch_actuator_init,
                                     .destroy_callback = mf_switch_actuator_destroy}};

#endif