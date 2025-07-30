idf_component_register(SRCS "NMEA2000.c"
                    INCLUDE_DIRS "."
                    REQUIRES 
                        ads1115_readings
                        driver
                    )
