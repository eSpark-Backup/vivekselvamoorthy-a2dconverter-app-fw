
#include <stdio.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include "ads1115_readings.h"
#include "driver/gpio.h"
#include "ble_handler.h"
#include "NMEA2000.h"
#include "esp_log.h"

#define I2C_ENABLE GPIO_NUM_33

#ifndef APP_CPU_NUM
#define APP_CPU_NUM PRO_CPU_NUM
#endif

 void i2c_Enable_GPIO() {
    gpio_reset_pin(I2C_ENABLE);
    gpio_set_direction(I2C_ENABLE, GPIO_MODE_OUTPUT);
    gpio_set_level(I2C_ENABLE, 1);
}

void ads111x_test(void *pvParameters)
{
    
    init_ads1115();
    BLEinit();
    NMEA_init();
    while (1) {
        readSCL();
        readGND();
        readVCC();
        
        ESP_LOGI("MAIN" , "VOLT_SIG = %d  ,SWITCH = %d ,tach_value = %d ,tilt_trim_value = %d ,oil_press_value = %d PSI ,coolant_value = %d ,warning = %d, fuel_1 = %d %%, fuel_2 = %d %% ,starter= %d" ,
            VOLT_SIG ,SWITCH ,tach_value ,tilt_trim_value ,oil_press_value ,coolant_value ,warning ,fuel_1 ,fuel_2 ,starter);

        vTaskDelay(pdMS_TO_TICKS(500));
    }
}

void app_main()
{
    i2c_Enable_GPIO();
    ESP_ERROR_CHECK(i2cdev_init());
    xTaskCreatePinnedToCore(ads111x_test, "ads111x_test", 4096, NULL, 5, NULL, APP_CPU_NUM);
}


