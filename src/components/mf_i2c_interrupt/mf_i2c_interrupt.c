#include "mf_i2c_interrupt.h"
#include "mf_gpio_interrupt.h"
#include "mf_i2c_device.h"
#include "mf_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
typedef struct {
    mf_device_t dev;
    mf_component_notify_t cb;
} i2c_interrupt_internal_t;

static xQueueHandle gpio_evt_queue = NULL;

static i2c_interrupt_internal_t devices[MF_I2C_MAX_DEVICES];
static void intr_handler(void){
    uint8_t rand = 0;
        xQueueSendFromISR(gpio_evt_queue, &rand, NULL);
   
}

static void gpio_task_example(void* arg)
{
    uint8_t rand;
    for(;;) {
        if(xQueueReceive(gpio_evt_queue, &rand, portMAX_DELAY)) {
            PRINT("TEST");
             uint8_t message = 'i';
             //TODO: This could be improved by sending broadcast and receiving id of who responds
             // In i2c broadcast is the address 0
            for(uint8_t i = 0; i < MF_I2C_MAX_DEVICES; i++){
                if(devices[i].dev != MF_DEVICE_INVALID){
                    uint8_t val;
                    
                    mf_i2c_send_and_receive_message(devices[i].dev, &message, sizeof(message),&val ,sizeof(val)); //TODO: I2c is not thread safe, implement mutex in mf_i2c
                    if(val == 0xFF){
                        PRINT("%x", val);
                        devices[i].cb();
                        break;
                    }
                }
            }
        }
    }
}

uint8_t mf_i2c_interrupt_init(uint8_t* pins, uint8_t len){
    mf_gpio_interrupt_init();
    //create a queue to handle gpio event from isr
    gpio_evt_queue = xQueueCreate(10, sizeof(uint8_t));
    xTaskCreate(gpio_task_example, "gpio_task_example", 2048, NULL, 10, NULL);

    for(uint8_t i=0; i < MF_I2C_MAX_DEVICES; i++){
        devices[i].dev = MF_DEVICE_INVALID;
    }

    for(uint8_t i=0; i < len; i++){
        mf_gpio_interrupt_register_isr(pins[i], intr_handler);
    }

    return 0;
}

uint8_t mf_i2c_interrupt_register(mf_device_t dev, mf_component_notify_t cb){
    uint8_t i;
    for(i =0; i < MF_I2C_MAX_DEVICES; i++){
        if(devices[i].dev == MF_DEVICE_INVALID)
            break;
    }

    if(i == MF_I2C_MAX_DEVICES)
        return 1;

    devices[i].dev = dev;
    devices[i].cb = cb;
    return 0;
}