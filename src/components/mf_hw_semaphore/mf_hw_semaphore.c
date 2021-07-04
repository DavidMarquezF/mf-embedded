#include "mf_hw_semaphore.h"
#include "mf_i2c_device.h"
#include "mf_log.h"

uint8_t mf_hw_semaphore_init(void){
    return mf_i2c_device_get_device(MF_DEVICE_ID_SEMAPHORE) == MF_I2C_INVALID_DEVICE ? 1 : 0;  
}
uint8_t mf_hw_semaphore_set_value(bool green, bool yellow, bool red){
    uint8_t value;

    value = ((yellow ? 1 : 0) << 0) |
        ((red ? 1 : 0) << 1) |
        (( green ? 1 : 0) << 2);

    return mf_i2c_device_set_value(MF_DEVICE_ID_SEMAPHORE, &value, sizeof(value));// It's important to note that it will receive 4 bytes representing the float
}
uint8_t mf_hw_semaphore_destroy(void){
    return 0;
}