idf_component_register(SRCS "mf_i2c_interrupt.c"
                    INCLUDE_DIRS "../include"
                    PRIV_REQUIRES mf_gpio_interrupt mf_i2c_device)
