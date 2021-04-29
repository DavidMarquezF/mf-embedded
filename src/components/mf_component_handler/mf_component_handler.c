#include "mf_component_handler.h"
#include "mf_log.h"

// config with the parameters that OCF doesn't store, we don't need a lot of the information that is necessary during oc registration
typedef struct{
    mf_component_init_t init_callback;
    mf_component_destroy_t destroy_callback;
} mf_component_internal_t;

static mf_component_internal_t components[MF_NUM_COMPONENTS];
static uint8_t current_component_index;

uint8_t mf_component_handler_init(void){
    current_component_index = 0;
    return 0;
}

uint8_t mf_component_handler_register(mf_component_config_t config)
{
    config.init_callback();
    PRINT("\n\nRegistering MF component: %s\n", config.name);
    oc_resource_t *res = oc_new_resource(config.name, config.url, 1, 0);
    config.create_resource_callback(res);
    oc_resource_set_discoverable(res, true);
    /* periodic observable                                                                                                                                                                                   
     to be used when one wants to send an event per time slice                                                                                                                                             
     period is 1 second */
   // oc_resource_set_periodic_observable(res, 1);
    /* set observable                                                                                                                                                                                        
     events are send when oc_notify_observers(oc_resource_t *resource) is called.                                                                                                                          
    this function must be called when the value changes, preferable on an interrupt when something is read from the hardware. */
    /*oc_resource_set_observable(res_temperature, true); */
#ifdef OC_CLOUD
PRINT("\tAdding to cloud...\n");
    oc_cloud_add_resource(res);
#endif

    oc_add_resource(res);

    mf_component_internal_t * comp = &components[current_component_index++];
    comp->destroy_callback = config.destroy_callback;
    comp->init_callback = config.init_callback;

    return 0;
}


uint8_t mf_component_handler_init_components(void){
    PRINT("\n\nInitializing MF components\n");

    bool error = false;
    for(int i = 0; i < current_component_index; i++)
        error |= components[i].init_callback() != 0;
    
    return error ? 1 : 0;
}

uint8_t mf_component_handler_destroy_components(void){
    PRINT("\n\nDestroying MF components\n");
    bool error = false;
    for(int i = 0; i < current_component_index; i++)
        error |= components[i].destroy_callback() != 0;
    
    return error ? 1 : 0;
}
