#include "mf_hw_ultrasound.h"
#include "mf_spi.h"
#include "mf_log.h"


uint8_t mf_hw_ultrasound_init(void){
    return mf_spi_init(); // TODO: Move this in the main, because if we have multiple SPI modules it will recall init multiple times. Here only the device should be registered
}
uint8_t mf_hw_ultrasound_get_value(float* ultrasound_value){
        PRINT("Sending data");

    uint8_t ret = mf_spi_send_message('g', NULL, 0);

    if(ret == 0){  
        ret = mf_spi_receive_message(ultrasound_value, sizeof(float)*8); // It's important to note that it will receive 4 bytes representing the float
    }

    return ret;
}
uint8_t mf_hw_ultrasound_destroy(void){
    return 0;
}