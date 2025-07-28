idf_component_register(SRCS "ads1115_readings.c"
                    INCLUDE_DIRS "."
                    REQUIRES 
                        ads111x
                        ble_services
                    )
