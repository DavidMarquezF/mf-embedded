idf_component_register(SRCS "mf_main.c"
                    INCLUDE_DIRS "../include"
                    PRIV_REQUIRES mf_component_handler mf_temp mf_ultrasound mf_switch_actuator mf_discovery)
