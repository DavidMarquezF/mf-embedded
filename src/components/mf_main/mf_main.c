#include "mf_main.h"
#include "oc_api.h"
#include "oc_core_res.h"
#include "mf_component_handler.h"
#include "mf_discovery.h"
#include "mf_updates_handler.h"
#include "mf_power.h"
#include "mf_i2c_interrupt.h"
#include "mf_spi.h"
#include "mf_i2c.h"
#include "mf_spi_device.h"
#include "mf_i2c_device.h"
#include "mf_delay.h"


void test(){
  PRINT("TES!!!!!!T");
}


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
    mf_power_init();
     mf_component_handler_init_components();
      #include "mf_hw_temp.h"
      float val;
    mf_hw_temp_get_value(&val);

    #include "mf_hw_button.h"
    PRINT("Button %d",mf_hw_button_init(test));
    PRINT("CURRENT TEMPERATURE: %f", val);
    #include "mf_hw_semaphore.h"
    mf_hw_semaphore_init();
    mf_hw_semaphore_set_value(false, true,true);
    return 0;

}
uint8_t mf_main_destroy_components(void){
    return mf_component_handler_destroy_components();
}

void mf_main_cloud_login(void){
    mf_updates_handler_cloud_login();
}


uint8_t mf_main_init(void){
      mf_updates_handler_init();

    mf_updates_handler_init_check_if_updated();
    assert(mf_power_init() == 0);
    mf_power_enable_modules(true);
  mf_delay_ms(1000);
  uint8_t spi_enable_pins[MF_SPI_MAX_DEVICES] = {18, 16, 15};  
  assert(mf_spi_init(spi_enable_pins) == 0);
  assert(mf_i2c_init() == 0);
  assert(mf_spi_device_discover_devices()==0);
  assert(mf_i2c_device_discover_devices()==0);
  uint8_t i2c_pins[MF_I2C_MAX_DEVICES] = {
    32,33,25
  };
  assert(mf_i2c_interrupt_init(i2c_pins, MF_I2C_MAX_DEVICES)==0);

  return 0;
  
}