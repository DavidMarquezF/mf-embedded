#ifndef _MF_MAIN_H_
#define _MF_MAIN_H_
#include <stdint.h>
void mf_main_register_resources(void);
uint8_t mf_main_init_components(void);
uint8_t mf_main_destroy_components(void);
void mf_main_cloud_login(void);
#endif