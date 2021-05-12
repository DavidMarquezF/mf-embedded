#include "mf_main.h"
#include "oc_api.h"
#include "oc_core_res.h"
#include "mf_component_handler.h"
#include "mf_discovery.h"
#include "mf_updates_handler.h"


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

    #ifdef OC_SOFTWARE_UPDATE
    // It is important that the sw and pstat resources are exposed to the cloud, so that updates can be triggered both schedueled and manually from there
    oc_cloud_add_resource(oc_core_get_resource_by_index(OCF_SW_UPDATE, 0));
    oc_cloud_add_resource(oc_core_get_resource_by_index(OCF_SEC_PSTAT, 0));
    #endif
    
    mf_discovery_register_resource(0);
   // return;
    
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

void mf_main_cloud_login(void){
    mf_updates_handler_cloud_login();
}
