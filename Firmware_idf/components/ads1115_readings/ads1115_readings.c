// ads1115_readings.c

#include <stdio.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <ads111x.h>
#include "driver/gpio.h"
#include <string.h> 
#include "ble_handler.h"  
#include "ads1115_readings.h"
#include "esp_err.h"

#define I2C_PORT 0
#define FREQ_HZ 400000

int16_t VOLT_SIG = 0;
float voltage_by_ads;
int16_t SWITCH = 0;
int16_t tach_value = 0;
int16_t rpm_display = 0;
int16_t tilt_trim_value = 0;
int16_t oil_press_value = 0;
int16_t coolant_value = 0;
int16_t warning = 0;
int16_t fuel_1 = 0;
int16_t fuel_2 = 0;
int16_t starter= 0;
float starter_voltage= 0;
char fuel_sensor_type = 'A'; // 'A' for American, 'E' for European


#define CONFIG_EXAMPLE_I2C_MASTER_SCL GPIO_NUM_22
#define CONFIG_EXAMPLE_I2C_MASTER_SDA GPIO_NUM_21

// #define GAIN ADS111X_GAIN_2V048
#define GAIN ADS111X_GAIN_6V144
#define DATA_RATE ADS111X_DATA_RATE_128
#define MODE ADS111X_MODE_CONTINUOUS

i2c_dev_t devicesSCL;
i2c_dev_t devicesGND;
i2c_dev_t devicesVCC;

static float gain_val;

void init_ads1115()
{
    // gain_val = ads111x_gain_values[GAIN];

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

    // fuel_sensor_type = 'E'; // Example: Assign 'E' for European
}

void readSCL()
{
    // int16_t VOLT_SIG = 0;
    // int16_t SWITCH = 0;

    ads111x_set_input_mux(&devicesSCL, ADS111X_MUX_0_GND);
    vTaskDelay(pdMS_TO_TICKS(100));
    if (ads111x_get_value(&devicesSCL, &VOLT_SIG) == ESP_OK)
    {   
        // voltage_by_ads = (VOLT_SIG / 32767.0) * 6.15f;
        // voltage_by_ads = voltage_by_ads * 1.4f ;
        voltage_by_ads = (VOLT_SIG * 6.144f) / 32767.0f;  // ADS voltage
        // voltage_by_ads = voltage_by_ads * ((39.0f + 100.0f) / 100.0f); // undo divider
        // voltage_by_ads = voltage_by_ads * 3.57f;
        // printf("AIN0 (VOLT Signal) value: %d, voltage: %.02f volts\n", VOLT_SIG, voltage);
        if (voltage_by_ads < 1.5f){
            voltage_by_ads = voltage_by_ads * 3.57f;
        }
        else if(voltage_by_ads < 2.0f){
            voltage_by_ads = voltage_by_ads * 3.67f;
        }
        else if(voltage_by_ads < 2.5f){
            voltage_by_ads = voltage_by_ads * 3.87f;
        }
        else if(voltage_by_ads < 2.72f){
            voltage_by_ads = voltage_by_ads * 4.4f;
        }
        else{
            voltage_by_ads = voltage_by_ads * 4.85f;
        }
    }

    ads111x_set_input_mux(&devicesSCL, ADS111X_MUX_1_GND);
    vTaskDelay(pdMS_TO_TICKS(100));
    if (ads111x_get_value(&devicesSCL, &SWITCH) == ESP_OK)
    {
        // printf("AIN1 Switch : %d\n", SWITCH);
    }
}

typedef struct {
  float percent;
  float voltage;
} Lookup; 

// Calibration table 


// --- North American style (240 Ω → 33 Ω) ---
// Data of Ohms -> ADC -> Voltage
// 240 -> 20214 adc -> 1.20 V 
// 219 -> 18756 adc -> 1.172 V
// 198.6 -> 18007 adc -> 1.124 V
// 177.9 ->  16307 adc ->  1.023 V
// 157.2 ->  13863 adc -> 0.871 V
// 136.5 ->  11640 adc -> 0.732 V
// 115.8 ->  9414 adc -> 0.592 V
// 95.1 ->  7815 adc -> 0.491 V
// 74.4 -> 5854 adc -> 0.366 V
// 53.7 -> 3861 adc -> 0.242 V
// 33.0 -> 2003 adc -> 0.126 V 


// --- European/VDO style (180 Ω → 10 Ω) ---

// 180 -> 106544 adc -> 1.037 voltage
// 163 -> 14884 adc -> 0.933 voltage
// 146 -> 12921 adc -> 0.813 voltage
// 129 -> 11127 adc -> 0.698 voltage
// 112 -> 9603 adc -> 0.604 voltage
// 95 -> 7981 adc -> 0.502 voltage
// 78 -> 6387 adc -> 0.400 voltage
// 61 -> 4432 adc -> 0.278 voltage
// 44 -> 3140 adc -> 0.196 voltage
// 27 -> 1487 adc -> 0.093 voltage
// 10 -> 33 adc -> 0.004 voltage


// Temp-Typical marine range:
// ~ 240 Ω at cold (around 40 °C / 100 °F)
// ~ 33 Ω at hot (about 120 °C / 250 °F)

/*
240 40

33 120



240 40


33 120


240-33 = 207
40-120 = -80
207/80 = 2.5875 ohms per degree C

40 degrees C = 240 ohms
45 degrees C = 227.0625 ohms
50 degrees C = 214.125 ohms
55 degrees C = 201.1875 ohms
60 degrees C = 188.25 ohms
65 degrees C = 175.3125 ohms
70 degrees C = 162.375 ohms
75 degrees C = 149.4375 ohms
80 degrees C = 136.5 ohms
85 degrees C = 123.5625 ohms
90 degrees C = 110.625 ohms
95 degrees C = 97.6875 ohms
100 degrees C = 84.75 ohms
105 degrees C = 71.8125 ohms
110 degrees C = 58.875 ohms
115 degrees C = 45.9375 ohms
120 degrees C = 33 ohms


240 ohms = 40 degrees C
219 ohms = 48.12 degrees C
198.6 ohms = 56.25 degrees C
177.9 ohms = 64.375 degrees C
157.2 ohms = 72.5 degrees C
136.5 ohms = 80.625 degrees C
115.8 ohms = 88.75 degrees C
95.1  ohms = 96.875 degrees C
74.4  ohms = 105 degrees C
53.7  ohms = 113.125 degrees C
33.0  ohms = 120 degrees C

*/

Lookup fuelTable_1[] = { //
    {0, 20214 }, // Empty 0 psi on 20214 adc value // 240 ohms
    {10, 18756 },
    {20, 18007 },
    {30, 16307 },
    {40, 13863 },
    {50, 11640 },
    {60, 9414 },
    {70, 7815 },
    {80, 5854 },
    {90, 3861 },
    {100, 2003 } // Full 100 psi on 2003 adc value // 33 ohms
};

Lookup fuelTable_2[] = { //
  {0, 16572 }, // Empty 0% on 16572 adc value // 180 ohms
  {10, 14884 },
  {20, 12921 },
  {30, 11127 },
  {40, 9603 },
  {50, 7981 },
  {60, 6387 },
  {70, 4432 },
  {80, 3140 },
  {90, 1487 },
  {100, 33 }, // Full 100% on 33 adc value // 10 ohms
};

Lookup oil_pressure[] = { //
    {0, 20214 }, // Empty 0 psi on 20214 adc value
    {10, 18756 },
    {20, 18007 },
    {30, 16307 },
    {40, 13863 },
    {50, 11640 },
    {60, 9414 },
    {70, 7815 },
    {80, 5854 },
    {90, 3861 },
    {100, 2003 } // Full 100 psi on 2003 adc value
};

Lookup tilt_trim[] = { //
    {0, 20214 }, // Empty 0 % on 20214 adc value
    {10, 18756 },
    {20, 18007 },
    {30, 16307 },
    {40, 13863 },
    {50, 11640 },
    {60, 9414 },
    {70, 7815 },
    {80, 5854 },
    {90, 3861 },
    {100, 2003 } // Full 100 % on 2003 adc value
};

Lookup coolant_temp[] = { //
    {40, 20214 }, // 40 degrees C on  adc value // 240 ohms
    {48.12, 18756 },
    {56.25, 18007 },
    {64.375, 16307 },
    {72.5, 13863 },
    {80.625, 11640 },
    {88.75, 9414 },
    {96.875, 7815 },
    {105, 5854 },
    {113.125, 3861 },
    {120, 2003 } // 120 degrees C on 2003 adc value // 33 ohms
};

// Function to get fuel level percentage based on voltage reading
int16_t get_fuel_level(int16_t value) {
    if(fuel_sensor_type == 'A'){
        for (int i =0; i< sizeof(fuelTable_1)/sizeof(fuelTable_1[0])-1; i++){
            if (value <= fuelTable_1[i].voltage){
                float ratio = (value - fuelTable_1[i].voltage) / (fuelTable_1[i+1].voltage - fuelTable_1[i].voltage);
                return fuelTable_1[i].percent + ratio * (fuelTable_1[i+1].percent - fuelTable_1[i].percent);
            }

        }
    }
    else{
        for (int i =0; i< sizeof(fuelTable_2)/sizeof(fuelTable_2[0])-1; i++){
            if (value <= fuelTable_2[i].voltage){
                float ratio = (value - fuelTable_2[i].voltage) / (fuelTable_2[i+1].voltage - fuelTable_2[i].voltage);
                return fuelTable_2[i].percent + ratio * (fuelTable_2[i+1].percent - fuelTable_2[i].percent);
            }
        }
    }
   
    return -1;
}

int16_t get_oil_pressure(int16_t value){
    
    for (int i =0; i< sizeof(oil_pressure)/sizeof(oil_pressure[0])-1; i++){
        if (value <= oil_pressure[i].voltage){
            float ratio = (value - oil_pressure[i].voltage) / (oil_pressure[i+1].voltage - oil_pressure[i].voltage);
            return oil_pressure[i].percent + ratio * (oil_pressure[i+1].percent - oil_pressure[i].percent);
        }

    }
    return -1;    
}

int16_t get_tilt_trim(int16_t value){
    
    for (int i =0; i< sizeof(tilt_trim)/sizeof(tilt_trim[0])-1; i++){
        if (value <= tilt_trim[i].voltage){
            float ratio = (value - tilt_trim[i].voltage) / (tilt_trim[i+1].voltage - tilt_trim[i].voltage);
            return tilt_trim[i].percent + ratio * (tilt_trim[i+1].percent - tilt_trim[i].percent);
        }

    }
    return -1;    
}


int16_t get_coolant_temp(int16_t value){

    for (int i =0; i< sizeof(coolant_temp)/sizeof(coolant_temp[0])-1; i++){
        if (value <= coolant_temp[i].voltage){
            float ratio = (value - coolant_temp[i].voltage) / (coolant_temp[i+1].voltage - coolant_temp[i].voltage);
            return coolant_temp[i].percent + ratio * (coolant_temp[i+1].percent - coolant_temp[i].percent);
        }

    }
    return -1;
}


// int16_t get_tach_value(int16_t value){

//     for (int i =0; i< sizeof(tach_table)/sizeof(tach_table[0])-1; i++){
//         if (value <= tach_table[i].voltage){
//             float ratio = (value - tach_table[i].voltage) / (tach_table[i+1].voltage - tach_table[i].voltage);
//             return tach_table[i].percent + ratio * (tach_table[i+1].percent - tach_table[i].percent);
//         }

//     }
//     return -1;
// }



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
                case 1: tilt_trim_value = get_tilt_trim(val); break;
                // case 2: oil_press_value = val; break;
                case 2: oil_press_value = get_oil_pressure(val); break;
                // case 3: coolant_value = val; break;
                case 3: coolant_value = get_coolant_temp(val); break;
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
            // printf("AIN%d (%s) value: %d\n", i, labels[i], val);
            switch (i)
            {
                case 0: warning = val; break;
                case 1: fuel_1 = get_fuel_level(val); break;
                // case 1: fuel_1 = val; break;
                // case 1: fuel_1 = FUEL_PERCENT_NA_RAW(val); break;
                case 2: fuel_2 = get_fuel_level(val); break;
                // case 2: fuel_2 = FUEL_PERCENT_NA_RAW(val); break;
                
                case 3: {
                    starter = val; 
                    starter_voltage = (val * 6.144f) / 32767.0f;
                    starter_voltage = starter_voltage * 3.57f;
                }
                
                break;
                printf("AIN%d (%s) value: %d\n", i, labels[i], val);
            }
        }
    }
}
    
int get_sensor_data_json(char *buffer, size_t size)
{
    tach_value = rpm_display;
    return snprintf(buffer, size,
        "{"
        "\"VOLT_SIG\":%.2f,"
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
        voltage_by_ads, SWITCH, tach_value, tilt_trim_value,
        oil_press_value, coolant_value, warning,
        fuel_1, fuel_2, starter);
        
        /*
        {
            "VOLT_SIG": 13,
            "SWITCH": 1,
            "TACH": 2870,
            "TILT_TRM": 42,
            "OIL_PRES": 78,
            "COOLANT": 92,
            "WARNING": 0,
            "FUEL_1": 54,
            "FUEL_2": 31,
            "STARTER": 0
        }
  */
    //  return snprintf(buffer, size, "This is a test string from ESP32");
}

