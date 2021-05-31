#include "mf_spi_device.h"
#include "mf_spi.h"
#include "mf_log.h"

typedef enum {
    MF_SPI_CMD_GET='g',
    MF_SPI_CMD_DISCOVER='i'
} mf_command_t;

typedef struct {
    mf_device_t device;
    mf_spi_device_t spi_device;
} mf_spi_device_internal_t;

static mf_spi_device_internal_t devices[MF_SPI_MAX_DEVICES]; 

static uint8_t send_and_receive_only_cmd(mf_spi_device_t dev, mf_command_t cmd, void * out_value, size_t size) {
    return mf_spi_send_and_receive_message(dev, cmd, NULL, 0, out_value, size);
}

uint8_t mf_spi_device_discover_devices(void){
    PRINT("\nDiscovering SPI devices...\n");
    for(int i = 0; i < MF_SPI_MAX_DEVICES; i++) {
        PRINT("\tDev %d\n", i);
        mf_spi_device_t dev = mf_spi_device_from_index(i);
        uint8_t id;
        // If the id is still 0 it means that the SPI is nort available
        if(send_and_receive_only_cmd(dev, MF_SPI_CMD_DISCOVER, &id, sizeof(id)*8) == 0){
            if(id != 0){
                PRINT("\t\tId: %d\n", id);
                devices[i].device = id;
                devices[i].spi_device = dev;
            }
            else{
                PRINT("\t\tNo module available\n");
                devices[i].device = MF_DEVICE_INVALID;
            }
            
        }
        else{
            PRINT("ERROR DISCOVERING SPI\n");
            return 1;
        }
    }
    return 0;
}

uint8_t mf_spi_device_get_value(mf_device_t dev, void * out_result, size_t size){
    return send_and_receive_only_cmd(mf_spi_device_get_device(dev), MF_SPI_CMD_GET, out_result, size);
}

mf_spi_device_t mf_spi_device_get_device(mf_device_t dev){
    for(int i = 0; i < MF_SPI_MAX_DEVICES; i++){
        mf_spi_device_internal_t curr_dev = devices[i];
        if(curr_dev.device != MF_DEVICE_INVALID && curr_dev.device == dev)
            return curr_dev.spi_device;
    }
    return MF_SPI_INVALID_DEVICE;
}

mf_device_t mf_spi_device_get_device_from_index(uint8_t index){
    return devices[index].device;
}