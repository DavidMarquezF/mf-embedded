#include "mf_i2c_device.h"
#include "mf_log.h"

static mf_device_t devices[MF_I2C_MAX_DEVICES];

uint8_t mf_i2c_device_discover_devices(void){
    PRINT("\nDiscovering I2C devices...\n");
    uint8_t index=0;
    //TODO: Improve this by just including the existing i2c modules
    for(uint8_t i = 1; i < 127; i++){
        if(mf_i2c_send_message(i, NULL, 0) == 0){
            devices[index++] = i;
            PRINT("\tId: %d\n", i);
            if(index >= MF_I2C_MAX_DEVICES)
                break;
        }
    }

    for(uint8_t i = index; i < MF_I2C_MAX_DEVICES; i++){
        devices[i] = MF_I2C_INVALID_DEVICE;
    }
    return 0;
}
uint8_t mf_i2c_device_get_value(mf_device_t dev, void * out_result, size_t size){
    return mf_i2c_receive_message(dev, out_result, size);
}
uint8_t mf_i2c_device_set_value(mf_device_t dev, void * value, size_t size){
    return mf_i2c_send_message(dev, value, size);
}
mf_i2c_device_t mf_i2c_device_get_device(mf_device_t dev){
    for(uint8_t i = 0; i < MF_I2C_MAX_DEVICES; i++){
        if(devices[i] == dev)
            return dev;
    }
    return MF_I2C_INVALID_DEVICE;
}   

mf_device_t mf_i2c_device_get_device_from_index(uint8_t index){
    return devices[index];
}