#include "mf_main.h"
#include "oc_api.h"
#include "mf_component_handler.h"
#include "mf_discovery.h"

/**
* register all the resources to the stack
* this function registers all application level resources:
* - each resource path is bind to a specific function for the supported methods (GET, POST, PUT)
* - each resource is 
*   - secure
*   - observable
*   - discoverable 
*   - used interfaces, including the default interface.
*     default interface is the first of the list of interfaces as specified in the input file
*/
void mf_main_register_resources(void)
{
    mf_discovery_register_resource(0);

    PRINT("\nREGISTERING MF RESOURCES...\n");
    //Keep components inside scope
    #include "gen/components.h"

    mf_component_handler_init();
    for(int i = 0; i < MF_NUM_COMPONENTS; i++)
        mf_component_handler_register(generated_components[i]);
}


uint8_t mf_main_init_components(void){
    return mf_component_handler_init_components();
}
uint8_t mf_main_destroy_components(void){
    return mf_component_handler_destroy_components();
}
