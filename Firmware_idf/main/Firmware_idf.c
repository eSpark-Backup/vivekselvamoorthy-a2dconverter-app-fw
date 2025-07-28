// #include <stdio.h>
// #include <freertos/FreeRTOS.h>
// #include <freertos/task.h>
// #include <ads111x.h>
// #include <string.h>
// #include "driver/gpio.h"

// #define I2C_PORT 0
// #define I2C_ENABLE GPIO_NUM_33
// #define FREQ_HZ 400000

// #ifndef APP_CPU_NUM
// #define APP_CPU_NUM PRO_CPU_NUM
// #endif

// #define CONFIG_EXAMPLE_I2C_MASTER_SCL GPIO_NUM_22
// #define CONFIG_EXAMPLE_I2C_MASTER_SDA GPIO_NUM_21

// #define GAIN ADS111X_GAIN_2V048 //!< +-6.144V
// #define DATA_RATE ADS111X_DATA_RATE_128
// #define MODE ADS111X_MODE_CONTINUOUS
// #define CONFIG_EXAMPLE_DEV_COUNT 3

// i2c_dev_t devicesSCL = {
//     .port = I2C_PORT,
//     .addr = ADS111X_ADDR_SCL,
//     .cfg.sda_io_num =  CONFIG_EXAMPLE_I2C_MASTER_SDA,
//     .cfg.scl_io_num =  CONFIG_EXAMPLE_I2C_MASTER_SCL,
//     .cfg.sda_pullup_en = GPIO_PULLUP_ENABLE,
//     .cfg.scl_pullup_en = GPIO_PULLUP_ENABLE,
//     .cfg.master.clk_speed = FREQ_HZ
// };

// i2c_dev_t devicesGND = {
//     .port = I2C_PORT,
//     .addr = ADS111X_ADDR_GND,
//     .cfg.sda_io_num =  CONFIG_EXAMPLE_I2C_MASTER_SDA,
//     .cfg.scl_io_num =  CONFIG_EXAMPLE_I2C_MASTER_SCL,
//     .cfg.sda_pullup_en = GPIO_PULLUP_ENABLE,
//     .cfg.scl_pullup_en = GPIO_PULLUP_ENABLE,
//     .cfg.master.clk_speed = FREQ_HZ
// };

// i2c_dev_t devicesVCC = {
//     .port = I2C_PORT,
//     .addr = ADS111X_ADDR_VCC,
//     .cfg.sda_io_num =  CONFIG_EXAMPLE_I2C_MASTER_SDA,
//     .cfg.scl_io_num =  CONFIG_EXAMPLE_I2C_MASTER_SCL,
//     .cfg.sda_pullup_en = GPIO_PULLUP_ENABLE,
//     .cfg.scl_pullup_en = GPIO_PULLUP_ENABLE,
//     .cfg.master.clk_speed = FREQ_HZ
// };

// // Gain value
// static float gain_val;

// static void i2c_Enable_GPIO(){
//     gpio_reset_pin(I2C_ENABLE);
//     gpio_set_direction(I2C_ENABLE, GPIO_MODE_OUTPUT);
//     gpio_set_level(I2C_ENABLE, 1);
// }

// static void readSCL()
// {   
//     int16_t VOLT_SIG = 0;
//     int16_t SWITCH = 0;

//     // Read AIN0 (MUX_0_GND)
//     ads111x_set_input_mux(&devicesSCL, ADS111X_MUX_0_GND); 
//     vTaskDelay(pdMS_TO_TICKS(100));
//     if (ads111x_get_value(&devicesSCL, &VOLT_SIG) == ESP_OK)
//     {
//         float voltage = gain_val / ADS111X_MAX_VALUE * VOLT_SIG;
//         printf("AIN0 (VOLT Signal) value: %d, voltage: %.02f volts\n", VOLT_SIG, voltage);
//     }

//     // Read AIN1 (MUX_1_GND)
//     ads111x_set_input_mux(&devicesSCL, ADS111X_MUX_1_GND); 
//     vTaskDelay(pdMS_TO_TICKS(100));
//     if (ads111x_get_value(&devicesSCL, &SWITCH) == ESP_OK)
//     {
//         printf("AIN1 Switch : %d, \n", SWITCH);
//     }
// }

// static void readGND(){
//     int16_t TACH = 0;
//     int16_t TILT_TRIM = 0;
//     int16_t OIL_PRESS = 0;
//     int16_t COOLANT = 0;
//     ads111x_set_input_mux(&devicesGND, ADS111X_MUX_0_GND); 
//     vTaskDelay(pdMS_TO_TICKS(100));
//     if (ads111x_get_value(&devicesGND, &TACH) == ESP_OK)
//     {
//         // float voltage = gain_val / ADS111X_MAX_VALUE * TACH;
//         printf("AIN0 (TACH) value: %d \n", TACH);
//     }

//     // Read AIN1 (MUX_1_GND)
//     ads111x_set_input_mux(&devicesGND, ADS111X_MUX_1_GND); 
//     vTaskDelay(pdMS_TO_TICKS(100));
//     if (ads111x_get_value(&devicesGND, &TILT_TRIM) == ESP_OK)
//     {
//         printf("AIN1 TILT_TRIM : %d \n", TILT_TRIM);
//     }

//     // Read AIN2 (MUX_1_GND)
//     ads111x_set_input_mux(&devicesGND, ADS111X_MUX_2_GND); 
//     vTaskDelay(pdMS_TO_TICKS(100));
//     if (ads111x_get_value(&devicesGND, &OIL_PRESS) == ESP_OK)
//     {
//         printf("AIN1 OIL_PRESS : %d \n", OIL_PRESS);
//     }

//      // Read AIN3 (MUX_1_GND)
//     ads111x_set_input_mux(&devicesGND, ADS111X_MUX_3_GND); 
//     vTaskDelay(pdMS_TO_TICKS(100));
//     if (ads111x_get_value(&devicesGND, &COOLANT) == ESP_OK)
//     {
//         printf("AIN1 COOLANT : %d \n", COOLANT);
//     }

// }

// static void readVCC(){

//     int16_t WARNING = 0;
//     int16_t FUEL_1 = 0;
//     int16_t FUEL_2 = 0;
//     int16_t STARTER = 0;
//     ads111x_set_input_mux(&devicesVCC, ADS111X_MUX_0_GND); 
//     vTaskDelay(pdMS_TO_TICKS(100));
//     if (ads111x_get_value(&devicesVCC, &WARNING) == ESP_OK)
//     {
//         // float voltage = gain_val / ADS111X_MAX_VALUE * TACH;
//         printf("AIN0 (WARNING) value: %d \n", WARNING);
//     }

//     // Read AIN1 (MUX_1_GND)
//     ads111x_set_input_mux(&devicesVCC, ADS111X_MUX_1_GND); 
//     vTaskDelay(pdMS_TO_TICKS(100));
//     if (ads111x_get_value(&devicesVCC, &FUEL_1) == ESP_OK)
//     {
//         printf("AIN1 FUEL_1 : %d \n", FUEL_1);
//     }

//     // Read AIN2 (MUX_1_GND)
//     ads111x_set_input_mux(&devicesVCC, ADS111X_MUX_2_GND); 
//     vTaskDelay(pdMS_TO_TICKS(100));
//     if (ads111x_get_value(&devicesVCC, &FUEL_2) == ESP_OK)
//     {
//         printf("AIN1 FUEL_2 : %d \n", FUEL_2);
//     }

//      // Read AIN3 (MUX_1_GND)
//     ads111x_set_input_mux(&devicesVCC, ADS111X_MUX_3_GND); 
//     vTaskDelay(pdMS_TO_TICKS(100));
//     if (ads111x_get_value(&devicesVCC, &STARTER) == ESP_OK)
//     {
//         printf("AIN1 STARTER : %d \n", STARTER);
//     }

// }

// // Main task
// void ads111x_test(void *pvParameters)
// {
//     gain_val = ads111x_gain_values[GAIN];

//         ESP_ERROR_CHECK(ads111x_init_desc(&devicesSCL, ADS111X_ADDR_SCL, I2C_PORT, CONFIG_EXAMPLE_I2C_MASTER_SDA, CONFIG_EXAMPLE_I2C_MASTER_SCL));
//         ESP_ERROR_CHECK(ads111x_init_desc(&devicesGND, ADS111X_ADDR_GND, I2C_PORT, CONFIG_EXAMPLE_I2C_MASTER_SDA, CONFIG_EXAMPLE_I2C_MASTER_SCL));
//         ESP_ERROR_CHECK(ads111x_init_desc(&devicesVCC, ADS111X_ADDR_VCC, I2C_PORT, CONFIG_EXAMPLE_I2C_MASTER_SDA, CONFIG_EXAMPLE_I2C_MASTER_SCL));
//         ads111x_set_mode(&devicesSCL, MODE);    // Continuous conversion mode
//         ads111x_set_mode(&devicesGND, MODE); 
//         ads111x_set_mode(&devicesVCC, MODE); 
//         ads111x_set_data_rate(&devicesSCL, DATA_RATE); 
//         ads111x_set_data_rate(&devicesGND, DATA_RATE); 
//         ads111x_set_data_rate(&devicesVCC, DATA_RATE); 
//         ads111x_set_gain(&devicesSCL, GAIN);
//         ads111x_set_gain(&devicesGND, GAIN);
//         ads111x_set_gain(&devicesVCC, GAIN);

//     while (1)
//     {
        
//         readSCL();
//         readGND();
//         readVCC();
//         vTaskDelay(pdMS_TO_TICKS(500));
//     }
// }

// void app_main()
// {
//     i2c_Enable_GPIO();
//     ESP_ERROR_CHECK(i2cdev_init());
//     xTaskCreatePinnedToCore(ads111x_test, "ads111x_test", configMINIMAL_STACK_SIZE * 10, NULL, 5, NULL, APP_CPU_NUM);
// }

#include <stdio.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include "ads1115_readings.h"
#include "driver/gpio.h"
#include "ble_handler.h"

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
    while (1) {
        readSCL();
        readGND();
        readVCC();
        vTaskDelay(pdMS_TO_TICKS(500));
    }
}

void app_main()
{
    i2c_Enable_GPIO();
    ESP_ERROR_CHECK(i2cdev_init());
    xTaskCreatePinnedToCore(ads111x_test, "ads111x_test", 4096, NULL, 5, NULL, APP_CPU_NUM);
}


