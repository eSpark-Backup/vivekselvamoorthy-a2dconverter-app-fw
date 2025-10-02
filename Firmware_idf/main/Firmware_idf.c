
#include <stdio.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include "ads1115_readings.h"
#include "driver/gpio.h"
#include "ble_handler.h"
#include "NMEA2000.h"
#include "esp_timer.h"

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

        ESP_LOGI("MAIN" , "VOLT_SIG = %d  ,SWITCH = %d ,tach_value = %d ,tilt_trim_value = %d ,oil_press_value = %d PSI ,coolant_value = %d degrees C,warning = %d, fuel_1 = %d %%, fuel_2 = %d %% ,starter= %d" ,
            VOLT_SIG ,SWITCH ,tach_value ,tilt_trim_value ,oil_press_value ,coolant_value ,warning ,fuel_1 ,fuel_2 ,starter);

        vTaskDelay(pdMS_TO_TICKS(500));
    }
}


#define TAG "TACHO"

// GPIO where optocoupler output is connected
#define TACH_GPIO GPIO_NUM_32  

// Engine config
#define CYLINDERS 8
#define PULSES_PER_REV (CYLINDERS / 2)

static volatile uint32_t pulse_count = 0;

// ISR handler
static void IRAM_ATTR tach_isr_handler(void* arg) {
    pulse_count++;
}



void tachometer_task(void *arg) {
    uint32_t last_count = 0;
    uint64_t last_time = esp_timer_get_time();

    while (1) {
        vTaskDelay(pdMS_TO_TICKS(1000));  // every 1 sec

        int64_t now = esp_timer_get_time();
        int32_t pulses = pulse_count - last_count;
        last_count = pulse_count;

        // Frequency in Hz
        double elapsed_sec = (now - last_time) / 1000000.0;
        last_time = now;




          // Read digital GPIO level
        int gpio_level = gpio_get_level(TACH_GPIO);

         // Read raw tachometer value (pulse count)

        // Print both
        // ESP_LOGI(TAG, "GPIO=%d , raw=%d , pulses=%lu",
        //          gpio_level, raw_tach, (unsigned long)tach_get_pulses());


                 


        double freq = pulses / elapsed_sec;  // pulses per second                                               
        int32_t rpm = (int)((freq * 60.0) / PULSES_PER_REV);

        ESP_LOGI(TAG, "Pulses: %ld, Freq: %.2f Hz, RPM: %ld, Pulse Count: %ld", pulses, freq, rpm, pulse_count);
        ESP_LOGI(TAG, "TACH_VALUE: %d", gpio_level);
    }
}

void app_main()
{
    i2c_Enable_GPIO();
    ESP_ERROR_CHECK(i2cdev_init());
    xTaskCreatePinnedToCore(ads111x_test, "ads111x_test", 4096, NULL, 5, NULL, APP_CPU_NUM);



    // Configure GPIO as input with pull-up
    gpio_config_t io_conf = {
        .intr_type = GPIO_INTR_NEGEDGE, // falling edge
        .mode = GPIO_MODE_INPUT,
        .pin_bit_mask = (1ULL << TACH_GPIO),
        .pull_up_en = 1,
        .pull_down_en = 0
    };
    gpio_config(&io_conf);

    // Install ISR service
    gpio_install_isr_service(0);
    gpio_isr_handler_add(TACH_GPIO, tach_isr_handler, NULL);

    // Start task
    xTaskCreate(tachometer_task, "tachometer_task", 2048, NULL, 5, NULL);

}


