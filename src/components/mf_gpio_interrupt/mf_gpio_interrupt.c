#include "driver/gpio.h"

#include "mf_gpio_interrupt.h"

uint8_t mf_gpio_interrupt_init(void){
    return gpio_install_isr_service(0) == ESP_OK ? 0 : 1;
}
mf_gpio_id_t mf_gpio_interrupt_register_isr(uint8_t pin, mf_gpio_cb_t cb){

    gpio_config_t io_conf;
    //interrupt of rising edge
    io_conf.intr_type = GPIO_INTR_POSEDGE;
    //bit mask of the pins, use GPIO4/5 here
    io_conf.pin_bit_mask = 1ULL << pin;
    //set as input mode
    io_conf.mode = GPIO_MODE_INPUT;
    //enable pull-up mode
    io_conf.pull_up_en = 0;
    io_conf.pull_down_en = 0;
    if(gpio_config(&io_conf) != ESP_OK)
        return 1;

    gpio_isr_handler_add(pin, cb, NULL);

    return 0;
}
void mf_gpio_interrupt_destroy_isr(mf_gpio_id_t pin){
      gpio_isr_handler_remove(pin);
}