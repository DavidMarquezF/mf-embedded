#include "mf_discovery.h"
#include "mf_discovery_spi.h"

uint8_t mf_discovery_discover(mf_device_t* dev, uint8_t max_devices){
    mf_discovery_spi_init_discovery();
    int i = 0;
    // Discovers new spi devices. If it doesn't find any more devices it will stop 
    while(i < max_devices && mf_discovery_spi_discover_next(&dev[i]) == 0 && dev[i] != MF_DEVICE_INVALID){
        i++;
    }
    
    return 0;
}