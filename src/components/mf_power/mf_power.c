#include "driver/gpio.h"

#include "mf_power.h"
#include "mf_log.h"

#define POWER_PIN 13

uint8_t mf_power_init(void){
    gpio_config_t io_conf;
    //interrupt of rising edge
    //bit mask of the pins, use GPIO4/5 here
    io_conf.pin_bit_mask = 1ULL << POWER_PIN;
        io_conf.intr_type = GPIO_INTR_DISABLE;
    io_conf.mode = GPIO_MODE_OUTPUT;
    //enable pull-up mode
    io_conf.pull_up_en = 0;
    io_conf.pull_down_en = 0;
    if(gpio_config(&io_conf) != ESP_OK)
        return 1;
    return 0;
}

void mf_power_enable_modules(bool enable){
    PRINT("ENABLE MODULES: %s\n", enable ? "true" : "false");
    assert(gpio_set_level(POWER_PIN, enable) == 0);
}
