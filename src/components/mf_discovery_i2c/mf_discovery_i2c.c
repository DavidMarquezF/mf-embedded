#include "mf_discovery_i2c.h"
#include "mf_i2c_device.h"

#include "mf_log.h"
static uint8_t i2c_device_index;

uint8_t mf_discovery_i2c_init_discovery(void){
    i2c_device_index = 0;
    return 0;
}


uint8_t mf_discovery_i2c_discover_next(mf_device_t *device){
    // go through cs pin anc check if there is a device
    if(i2c_device_index >= MF_I2C_MAX_DEVICES){
        *device = MF_DEVICE_INVALID;
        return 0;
    }

    mf_device_t dev;
    while(i2c_device_index < MF_I2C_MAX_DEVICES && (dev = mf_i2c_device_get_device_from_index(i2c_device_index)) == MF_DEVICE_INVALID)
        i2c_device_index++;
    
    PRINT("Discovered I2C : %d %d", i2c_device_index, dev);
    if(i2c_device_index < MF_I2C_MAX_DEVICES){
        *device = dev;
        i2c_device_index++;
    }
    else
        *device = MF_DEVICE_INVALID;
    
    return 0;
}
