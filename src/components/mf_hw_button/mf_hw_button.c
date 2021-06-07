#include <stdbool.h>

#include "mf_hw_button.h"
#include "mf_i2c_device.h"
#include "mf_i2c_interrupt.h"

static bool active = true;
uint8_t mf_hw_button_init(mf_notify_cb_t change_cb){
    if(mf_i2c_device_get_device(MF_DEVICE_ID_BUTTON) == MF_I2C_INVALID_DEVICE)
        return 1;
    
    return mf_i2c_interrupt_register(MF_DEVICE_ID_BUTTON, change_cb);  
}
uint8_t mf_hw_button_get_value(bool*value){
    active = !active;
    *value = active;
    return 0;
    // No need to ask, it is an ON-(OFF) button it doesn't make sense, just toggle so that the server can observe a change
    // return mf_i2c_device_get_value(MF_DEVICE_ID_BUTTON, &value, sizeof(value));// It's important to note that it will receive 4 bytes representing the float
}
uint8_t mf_hw_button_destroy(void){
    return 0;
}