#include "mf_discovery_spi.h"
#include "mf_spi_device.h"

#include "mf_log.h"
static uint8_t spi_device_index;

uint8_t mf_discovery_spi_init_discovery(void){
    spi_device_index = 0;
    return 0;
}


uint8_t mf_discovery_spi_discover_next(mf_device_t *device){
    // go through cs pin anc check if there is a device
    if(spi_device_index >= MF_SPI_MAX_DEVICES){
        *device = MF_DEVICE_INVALID;
        return 0;
    }

    mf_device_t dev;
    while(spi_device_index < MF_SPI_MAX_DEVICES && (dev = mf_spi_device_get_device_from_index(spi_device_index)) == MF_DEVICE_INVALID)
        spi_device_index++;
    
    PRINT("Discovered SPI : %d %d", spi_device_index, dev);
    if(spi_device_index < MF_SPI_MAX_DEVICES){
        *device = dev;
        spi_device_index++;
    }
    else
        *device = MF_DEVICE_INVALID;
    
    return 0;
}
