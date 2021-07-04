idf_component_register(SRCS "mf_main.c"
                    INCLUDE_DIRS "../include"
                    PRIV_REQUIRES mf_component_handler iotivity mf_discovery mf_power mf_spi mf_i2c mf_spi_device mf_i2c_device mf_updates_handler mf_delay mf_i2c_interrupt mf_hw_temp mf_hw_button)
