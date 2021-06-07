#ifndef _MF_DEVICE_H_
#define _MF_DEVICE_H_
#include <stdint.h>

#define MF_DEVICE_INVALID 255

typedef uint8_t mf_device_t;


/*-------------SPI DEVICES--------------*/
#define MF_DEVICE_ID_ULTRASOUND 1
#define MF_DEVICE_ID_TEMP 2
#define MF_DEVICE_ID_SEMAPHORE 3
#define MF_DEVICE_ID_BUTTON 4
#endif