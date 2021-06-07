#ifndef _MF_GPIO_INTERRUPTS_H_
#define _MF_GPIO_INTERRUPTS_H_
#include <stdint.h>
typedef uint8_t mf_gpio_id_t;
typedef void (*mf_gpio_cb_t)(void);

uint8_t mf_gpio_interrupt_init(void);
mf_gpio_id_t mf_gpio_interrupt_register_isr(uint8_t pin, mf_gpio_cb_t cb);
void mf_gpio_interrupt_destroy_isr(mf_gpio_id_t pin);
#endif