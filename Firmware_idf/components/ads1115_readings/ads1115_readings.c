// ads1115_readings.c

#include <stdio.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <ads111x.h>
#include "driver/gpio.h"
#include <string.h> 
#include "ble_handler.h"  
#include "ads1115_readings.h"

#define I2C_PORT 0
#define FREQ_HZ 400000

int16_t VOLT_SIG = 0;
int16_t SWITCH = 0;
int16_t tach_value = 0;
int16_t tilt_trim_value = 0;
int16_t oil_press_value = 0;
int16_t coolant_value = 0;
int16_t warning = 0;
int16_t fuel_1 = 0;
int16_t fuel_2 = 0;
int16_t starter= 0;

#define CONFIG_EXAMPLE_I2C_MASTER_SCL GPIO_NUM_22
#define CONFIG_EXAMPLE_I2C_MASTER_SDA GPIO_NUM_21

#define GAIN ADS111X_GAIN_2V048
#define DATA_RATE ADS111X_DATA_RATE_128
#define MODE ADS111X_MODE_CONTINUOUS

i2c_dev_t devicesSCL;
i2c_dev_t devicesGND;
i2c_dev_t devicesVCC;

static float gain_val;

void init_ads1115()
{
    gain_val = ads111x_gain_values[GAIN];

    // Initialize descriptors
    devicesSCL = (i2c_dev_t){
        .port = I2C_PORT,
        .addr = ADS111X_ADDR_SCL,
        .cfg = {
            .sda_io_num = CONFIG_EXAMPLE_I2C_MASTER_SDA,
            .scl_io_num = CONFIG_EXAMPLE_I2C_MASTER_SCL,
            .sda_pullup_en = GPIO_PULLUP_ENABLE,
            .scl_pullup_en = GPIO_PULLUP_ENABLE,
            .master.clk_speed = FREQ_HZ
        }
    };

    devicesGND = devicesSCL;
    devicesGND.addr = ADS111X_ADDR_GND;

    devicesVCC = devicesSCL;
    devicesVCC.addr = ADS111X_ADDR_VCC;

    ESP_ERROR_CHECK(ads111x_init_desc(&devicesSCL, devicesSCL.addr, I2C_PORT, CONFIG_EXAMPLE_I2C_MASTER_SDA, CONFIG_EXAMPLE_I2C_MASTER_SCL));
    ESP_ERROR_CHECK(ads111x_init_desc(&devicesGND, devicesGND.addr, I2C_PORT, CONFIG_EXAMPLE_I2C_MASTER_SDA, CONFIG_EXAMPLE_I2C_MASTER_SCL));
    ESP_ERROR_CHECK(ads111x_init_desc(&devicesVCC, devicesVCC.addr, I2C_PORT, CONFIG_EXAMPLE_I2C_MASTER_SDA, CONFIG_EXAMPLE_I2C_MASTER_SCL));

    ads111x_set_mode(&devicesSCL, MODE);
    ads111x_set_mode(&devicesGND, MODE);
    ads111x_set_mode(&devicesVCC, MODE);

    ads111x_set_data_rate(&devicesSCL, DATA_RATE);
    ads111x_set_data_rate(&devicesGND, DATA_RATE);
    ads111x_set_data_rate(&devicesVCC, DATA_RATE);

    ads111x_set_gain(&devicesSCL, GAIN);
    ads111x_set_gain(&devicesGND, GAIN);
    ads111x_set_gain(&devicesVCC, GAIN);
}

void readSCL()
{
    // int16_t VOLT_SIG = 0;
    // int16_t SWITCH = 0;

    ads111x_set_input_mux(&devicesSCL, ADS111X_MUX_0_GND);
    vTaskDelay(pdMS_TO_TICKS(100));
    if (ads111x_get_value(&devicesSCL, &VOLT_SIG) == ESP_OK)
    {
        float voltage = gain_val / ADS111X_MAX_VALUE * VOLT_SIG;
        printf("AIN0 (VOLT Signal) value: %d, voltage: %.02f volts\n", VOLT_SIG, voltage);
    }

    ads111x_set_input_mux(&devicesSCL, ADS111X_MUX_1_GND);
    vTaskDelay(pdMS_TO_TICKS(100));
    if (ads111x_get_value(&devicesSCL, &SWITCH) == ESP_OK)
    {
        printf("AIN1 Switch : %d\n", SWITCH);
    }
}

void readGND()
{
    int16_t val = 0;
    ads111x_mux_t muxes[] = {ADS111X_MUX_0_GND, ADS111X_MUX_1_GND, ADS111X_MUX_2_GND, ADS111X_MUX_3_GND};
    const char *labels[] = {"TACH", "TILT_TRIM", "OIL_PRESS", "COOLANT"};

    for (int i = 0; i < 4; ++i)
    {
        ads111x_set_input_mux(&devicesGND, muxes[i]);
        vTaskDelay(pdMS_TO_TICKS(100));
        if (ads111x_get_value(&devicesGND, &val) == ESP_OK)
        {
            switch (i)
            {
                case 0: tach_value = val; break;
                case 1: tilt_trim_value = val; break;
                case 2: oil_press_value = val; break;
                case 3: coolant_value = val; break;
            }
            printf("AIN%d (%s) value: %d\n", i, labels[i], val);
        }
    }
}

void readVCC()
{
    int16_t val = 0;
    ads111x_mux_t muxes[] = {ADS111X_MUX_0_GND, ADS111X_MUX_1_GND, ADS111X_MUX_2_GND, ADS111X_MUX_3_GND};
    const char *labels[] = {"WARNING", "FUEL_1", "FUEL_2", "STARTER"};

    for (int i = 0; i < 4; ++i)
    {
        ads111x_set_input_mux(&devicesVCC, muxes[i]);
        vTaskDelay(pdMS_TO_TICKS(100));
        if (ads111x_get_value(&devicesVCC, &val) == ESP_OK)
        {
            printf("AIN%d (%s) value: %d\n", i, labels[i], val);
            switch (i)
            {
                case 0: warning = val; break;
                case 1: fuel_1 = val; break;
                case 2: fuel_2 = val; break;
                case 3: starter = val; break;
            }
        }
    }
}

int get_sensor_data_json(char *buffer, size_t size)
{
    return snprintf(buffer, size,
        "{"
        "\"VOLT_SIG\":%d,"
        "\"SWITCH\"  :%d,"
        "\"TACH\"    :%d,"
        "\"TILT_TRM\":%d,"
        "\"OIL_PRES\":%d,"
        "\"COOLANT\" :%d,"
        "\"WARNING\" :%d,"
        "\"FUEL_1\"  :%d,"
        "\"FUEL_2\"  :%d,"
        "\"STARTER\" :%d"
        "}",
        VOLT_SIG, SWITCH, tach_value, tilt_trim_value,
        oil_press_value, coolant_value, warning,
        fuel_1, fuel_2, starter);
    //  return snprintf(buffer, size, "This is a test string from ESP32.");
}

