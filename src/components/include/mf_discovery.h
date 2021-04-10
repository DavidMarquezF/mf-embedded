#ifndef _MF_DISCOVERY_H_
#define _MF_DISCOVERY_H_
#include <stdint.h>
#include "mf_device.h"


uint8_t mf_discovery_discover(mf_device_t* dev, uint8_t max_devices);

#endif
