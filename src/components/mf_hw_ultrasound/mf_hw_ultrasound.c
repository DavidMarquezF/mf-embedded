#include "mf_hw_ultrasound.h"
#include "mf_spi.h"
#include "mf_log.h"

static mf_spi_device_t device;

uint8_t mf_hw_ultrasound_init(void){
    device = mf_spi_add_device(5);
    return  device == MF_INVALID_SPI_DEVICE;// TODO: Move this in the main, because if we have multiple SPI modules it will recall init multiple times. Here only the device should be registered
}
uint8_t mf_hw_ultrasound_get_value(float* ultrasound_value){
        PRINT("Sending data");

    uint8_t ret = mf_spi_send_message(device, 'g', NULL, 0);

    if(ret == 0){  
        ret = mf_spi_receive_message(device, ultrasound_value, sizeof(float)*8); // It's important to note that it will receive 4 bytes representing the float
    }

    return ret;
}
uint8_t mf_hw_ultrasound_destroy(void){
    return 0;
}