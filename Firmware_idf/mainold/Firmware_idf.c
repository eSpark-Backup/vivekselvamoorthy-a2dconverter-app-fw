

// #include <stdio.h>
// #include "freertos/FreeRTOS.h"
// #include "freertos/task.h"
// #include "driver/gpio.h"
// #include "ADS1115.h"
// #include "driver/i2c.h"
// #include "esp_log.h"

// #define I2C_PORT I2C_NUM_0
// #define SDA_PIN GPIO_NUM_21
// #define SCL_PIN GPIO_NUM_22
// #define I2C_ENABLE GPIO_NUM_33

// #define DEVICE_SCL 0x4B
// #define DEVICE_GND 0x48
// #define DEVICE_VCC 0x49

// static const char *TAG = "ads1115";

// int16_t VOLT_SIG = 0;
// int16_t SWITCH = 0;
// int16_t TECH = 0;
// int16_t TILT_TRIM = 0; 
// int16_t OIL_PRESS = 0;
// int16_t COOLENT = 0;
// int16_t WARNING = 0;
// int16_t FUEL_1 = 0;
// int16_t FUEL_2 = 0;
// int16_t STARTER = 0;


// ads1115_t ads1115_scl = {
//   .reg_cfg =  ADS1115_CFG_LS_DR_250SPS  ,          
//   .dev_addr = DEVICE_SCL,
// };

// ads1115_t ads1115_gnd = {
//   .reg_cfg = ADS1115_CFG_LS_DR_250SPS,
//   .dev_addr = DEVICE_GND,
// };

// ads1115_t ads1115_vcc = {
//   .reg_cfg = ADS1115_CFG_LS_DR_250SPS,
//   .dev_addr = DEVICE_VCC,
// };


// static void i2c_Enable_GPIO(){
//     gpio_reset_pin(I2C_ENABLE);
//     gpio_set_direction(I2C_ENABLE, GPIO_MODE_OUTPUT);
//     gpio_set_level(I2C_ENABLE, 1);
// }
// // I2C Master Initialization
// static esp_err_t i2c_master_init(void)
// {
//     i2c_config_t i2c_conf = {
//         .mode = I2C_MODE_MASTER,
//         .sda_io_num = SDA_PIN,
//         .scl_io_num = SCL_PIN,
//         .sda_pullup_en = GPIO_PULLUP_ENABLE,
//         .scl_pullup_en = GPIO_PULLUP_ENABLE,
//         .master = {
//             .clk_speed = 100000  // 100kHz
//         },
//         .clk_flags = 0
//     };

//     ESP_ERROR_CHECK(i2c_param_config(I2C_PORT, &i2c_conf));

//     esp_err_t ret = i2c_driver_install(I2C_PORT, I2C_MODE_MASTER, 0, 0, 0);
//     if (ret != ESP_OK) {
//         ESP_LOGI(TAG, "Failed to install I2C driver: %s", esp_err_to_name(ret));
//     }

//     return ret;
// }

// uint16_t readAdsValue(uint8_t val){
//     ADS1115_set_multiplexer(val);
//     vTaskDelay(pdMS_TO_TICKS(100));
//     return ADS1115_get_conversion();
// }

// void reading_ads1115_values_SCL(){
//     ESP_ERROR_CHECK(ADS1115_initiate(&ads1115_scl));
//     ESP_LOGI(TAG, " ===============Reading ADS1115 (SCL)===================");
//     vTaskDelay(pdMS_TO_TICKS(10));
//     VOLT_SIG = readAdsValue(0x04);
//     SWITCH = readAdsValue(0x05);
//     ESP_LOGI(TAG, " AIN0 VOLT_SIG Value: %d", VOLT_SIG);
//     ESP_LOGI(TAG, " AIN0 SWITCH Value: %d", SWITCH);
// }

// void reading_ads1115_values_GND(){
//     ESP_ERROR_CHECK(ADS1115_initiate(&ads1115_gnd));
//     ESP_LOGI(TAG, " ===============Reading ADS1115 (GND)===================");
//     vTaskDelay(pdMS_TO_TICKS(10));
//     TECH = readAdsValue(0x04);
//     TILT_TRIM = readAdsValue(0x05);
//     OIL_PRESS = readAdsValue(0x06);
//     COOLENT = readAdsValue(0x07);

//     ESP_LOGI(TAG, " AIN0 TECH Value: %d", TECH);
//     ESP_LOGI(TAG, " AIN1 TILT_TRIM Value: %d", TILT_TRIM);
//     ESP_LOGI(TAG, " AIN2 OIL_PRESS Value: %d", OIL_PRESS);
//     ESP_LOGI(TAG, " AIN3 COOLENT Value: %d", COOLENT);
// }

// void reading_ads1115_values_VCC(){
//     ESP_ERROR_CHECK(ADS1115_initiate(&ads1115_vcc));
//     ESP_LOGI(TAG, " ===============Reading ADS1115 (VCC)===================");
//     vTaskDelay(pdMS_TO_TICKS(10));

//     WARNING = readAdsValue(0x04);
//     FUEL_1 = readAdsValue(0x05);
//     FUEL_2 = readAdsValue(0x06);
//     STARTER = readAdsValue(0x07);

//     ESP_LOGI(TAG, " AIN0 WARNING Value: %d", WARNING);
//     ESP_LOGI(TAG, " AIN1 FUEL_1 Value: %d", FUEL_1);
//     ESP_LOGI(TAG, " AIN2 FUEL_2 Value: %d", FUEL_2);
//     ESP_LOGI(TAG, " AIN3 STARTER Value: %d", STARTER);
// }

// void app_main(void)
// {
//     i2c_Enable_GPIO();
//     ESP_LOGI(TAG, "Calling I2C Init...");
//     ESP_ERROR_CHECK(i2c_master_init());
//     ESP_LOGI(TAG, "I2C Initialized");

//     while (1)
//     {  
//     reading_ads1115_values_SCL();
//     reading_ads1115_values_GND();
//     reading_ads1115_values_VCC();
    
//     vTaskDelay(pdMS_TO_TICKS(200));

//     }
// }


#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"
#include "adc111x.h"
#include "driver/i2c.h"
#include "esp_log.h"
#include <string.h>

#define I2C_PORT 0

#ifndef APP_CPU_NUM
#define APP_CPU_NUM PRO_CPU_NUM
#endif

#define GAIN ADS111X_GAIN_4V096 // +-4.096V

#define CONFIG_EXAMPLE_DEV_COUNT 3

// I2C addresses
static const uint8_t addr[CONFIG_EXAMPLE_DEV_COUNT] = {
    ADS111X_ADDR_GND,
    ADS111X_ADDR_VCC,
    ADS111X_ADDR_SCL
};

// Descriptors
static i2c_dev_t devices[CONFIG_EXAMPLE_DEV_COUNT];

// Gain value
static float gain_val;

static void measure(size_t n)
{
    // wait for conversion end
    bool busy;
    // do
    // {
    //     ads111x_is_busy(&devices[n], &busy);
    // }
    // while (busy);

    // Read result
    int16_t raw = 0;
    if (ads111x_get_value(&devices[n], &raw) == ESP_OK)
    {
        float voltage = gain_val / ADS111X_MAX_VALUE * raw;
        printf("[%u] Raw ADC value: %d, voltage: %.04f volts\n", n, raw, voltage);
    }
    else
        printf("[%u] Cannot read ADC value\n", n);
}

// Main task
void ads111x_test(void *pvParameters)
{
    gain_val = ads111x_gain_values[GAIN];

    // Setup ICs
    for (size_t i = 0; i < CONFIG_EXAMPLE_DEV_COUNT; i++)
    {
        ESP_ERROR_CHECK(ads111x_init_desc(&devices[i], addr[i], I2C_PORT, CONFIG_EXAMPLE_I2C_MASTER_SDA, CONFIG_EXAMPLE_I2C_MASTER_SCL));

        ESP_ERROR_CHECK(ads111x_set_mode(&devices[i], ADS111X_MODE_SINGLE_SHOT));    // Continuous conversion mode
        ESP_ERROR_CHECK(ads111x_set_data_rate(&devices[i], ADS111X_DATA_RATE_32)); // 32 samples per second
        ESP_ERROR_CHECK(ads111x_set_input_mux(&devices[i], ADS111X_MUX_0_GND));    // positive = AIN0, negative = GND
        ESP_ERROR_CHECK(ads111x_set_gain(&devices[i], GAIN));
    }

    while (1)
    {
        for (size_t i = 0; i < CONFIG_EXAMPLE_DEV_COUNT; i++)
            measure(i);

        vTaskDelay(pdMS_TO_TICKS(500));
    }
}

void app_main()
{
    // Init library
    ESP_ERROR_CHECK(i2cdev_init());

    // Clear device descriptors
    memset(devices, 0, sizeof(devices));

    // Start task
    xTaskCreatePinnedToCore(ads111x_test, "ads111x_test", configMINIMAL_STACK_SIZE * 8, NULL, 5, NULL, APP_CPU_NUM);
}

