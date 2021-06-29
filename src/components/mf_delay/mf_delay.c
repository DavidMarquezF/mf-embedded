#include "mf_delay.h"

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"


void mf_delay_ms(uint32_t ms){
    vTaskDelay(ms/portTICK_PERIOD_MS);
}