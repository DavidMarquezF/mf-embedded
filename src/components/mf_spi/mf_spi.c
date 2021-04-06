#include <string.h>

#include "mf_spi.h"
#include "driver/spi_master.h"
#include "driver/gpio.h"
#include "esp_err.h"
//To speed up transfers, every SPI transfer sends a bunch of lines. This define specifies how many. More means more memory use,
//but less overhead for setting up / finishing transfers. Make sure 240 is dividable by this.
#define PARALLEL_LINES 16

typedef struct {
    int16_t csPin;
    spi_device_handle_t device;
} mf_spi_device_internal_t;

static mf_spi_device_internal_t spi_devices[MF_SPI_MAX_DEVICES];

uint8_t mf_spi_init(void)
{
    for(int i = 0; i < MF_SPI_MAX_DEVICES; i++)
        spi_devices[i].csPin = MF_INVALID_SPI_DEVICE;

    esp_err_t ret;

    // Configure the bus
    spi_bus_config_t busconfig = {
        .miso_io_num = VSPI_IOMUX_PIN_NUM_MISO,
        .mosi_io_num = VSPI_IOMUX_PIN_NUM_MOSI,
        .sclk_io_num = VSPI_IOMUX_PIN_NUM_CLK,
        .quadwp_io_num = -1,
        .quadhd_io_num = -1,
        .max_transfer_sz = PARALLEL_LINES * 320 * 2 + 8};
    ret = spi_bus_initialize(VSPI_HOST, &busconfig, 0); // 0 means that it doesn't use DMA (Direct memory access)
    if (ret != ESP_OK)
        return 1;
    
    return 0;
}


static int find_available_device(void){
    int i;
    for(i = 0; i < MF_SPI_MAX_DEVICES; i++)
        if(spi_devices[i].csPin == MF_INVALID_SPI_DEVICE)
            break;
    return i == MF_SPI_MAX_DEVICES ? MF_INVALID_SPI_DEVICE : i;
}

static int find_device(mf_spi_device_t device){
    for(int i = 0; i < MF_SPI_MAX_DEVICES; i++)
        if(spi_devices[i].csPin == device)
            return i;
    return MF_INVALID_SPI_DEVICE;
}

mf_spi_device_t mf_spi_add_device(uint8_t enablepin){
//Enable pin example: VSPI_IOMUX_PIN_NUM_CS

    int available_device = find_available_device();
    if(available_device == MF_INVALID_SPI_DEVICE)
        return MF_INVALID_SPI_DEVICE;
    
    //Configure the specific device
    spi_device_interface_config_t devcfg = {
        .clock_speed_hz = (16 * 1000 * 1000) / 128, //Clock out at 16/ 128 MHz
        .mode = 0,                                  //SPI mode 0
        .flags = SPI_DEVICE_HALFDUPLEX,
        .spics_io_num = -1,                         //CS pin
        .queue_size = 7,                            //We want to be able to queue 7 transactions at a time
                                                    // .pre_cb = lcd_spi_pre_transfer_callback, //Specify pre-transfer callback to handle D/C line
        .command_bits= 8
    };

    //Attach the device to the SPI bus
    if (spi_bus_add_device(VSPI_HOST, &devcfg, &spi_devices[available_device].device) != ESP_OK)
        return MF_INVALID_SPI_DEVICE;

    gpio_config_t io_conf;
    // Disable interrupt
    io_conf.intr_type = GPIO_PIN_INTR_DISABLE;
    // set as output
    io_conf.mode = GPIO_MODE_OUTPUT;
    io_conf.pin_bit_mask = 1ULL << enablepin;
    // Disable pull down mode
    io_conf.pull_down_en = 0;
    // Disable pull up mode
    io_conf.pull_up_en = 0;
    if(gpio_config(&io_conf) != ESP_OK)
        return MF_INVALID_SPI_DEVICE;

    spi_devices[available_device].csPin = enablepin;
    
    return enablepin;
}

uint8_t mf_spi_send_message(mf_spi_device_t device, uint8_t cmd, uint8_t *message, size_t length)
{
    int devIndex = find_device(device);
    if(devIndex == MF_INVALID_SPI_DEVICE)
        return 1;
    mf_spi_device_internal_t dev = spi_devices[devIndex];
    
    // Enable SPI CS
    gpio_set_level(dev.csPin, 0);

    spi_transaction_t t;
    memset(&t, 0, sizeof(t));
    t.length = length;
    t.cmd = cmd;
    t.tx_buffer = message;
    esp_err_t ret = spi_device_polling_transmit(dev.device, &t);

    gpio_set_level(dev.csPin, 1);
    return ret == ESP_OK ? 0 : 1;
}

uint8_t mf_spi_receive_message(mf_spi_device_t device,void* receive_buffer, size_t length){
    int devIndex = find_device(device);
    if(devIndex == MF_INVALID_SPI_DEVICE)
        return 1;
    mf_spi_device_internal_t dev = spi_devices[devIndex];
    
    gpio_set_level(dev.csPin, 0);

    spi_transaction_t t;
    memset(&t, 0, sizeof(t));
    t.rxlength = length;
    t.rx_buffer = receive_buffer;
    esp_err_t ret = spi_device_polling_transmit(dev.device, &t);
    gpio_set_level(dev.csPin, 1);

    return ret == ESP_OK ? 0 : 1;
}
