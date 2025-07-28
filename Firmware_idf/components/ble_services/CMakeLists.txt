idf_component_register(SRCS "ble_handler.c"
                    INCLUDE_DIRS "."
                    REQUIRES 
                        bt
                        nvs_flash
                        ads1115_readings

                    )
