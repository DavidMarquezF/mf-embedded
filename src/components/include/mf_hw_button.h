#ifndef _MF_BUTTON_H_
#define _MF_BUTTON_H_
#include <stdint.h>
#include "mf_helpers.h"

uint8_t mf_hw_button_init(mf_notify_cb_t change_cb);
uint8_t mf_hw_button_get_value(bool* value);
uint8_t mf_hw_button_destroy(void);
#endif