#include "mf_hw_ultrasound.h"
#include "mf_spi_device.h"
#include "mf_log.h"


uint8_t mf_hw_ultrasound_init(void){
    return mf_spi_device_get_device(MF_DEVICE_ID_ULTRASOUND) == MF_SPI_INVALID_DEVICE ? 1 : 0;  
}
uint8_t mf_hw_ultrasound_get_value(float* ultrasound_value){
    return mf_spi_device_get_value(MF_DEVICE_ID_ULTRASOUND, ultrasound_value, sizeof(float)*8);// It's important to note that it will receive 4 bytes representing the float
}
uint8_t mf_hw_ultrasound_destroy(void){
    return 0;
}