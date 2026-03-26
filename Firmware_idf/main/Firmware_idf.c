
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

// GPIO where optocoupler output is connected
#define TACH_GPIO GPIO_NUM_32

// Engine config
#define CYLINDERS 8
#define PULSES_PER_REV (CYLINDERS / 2)   // V8 -> 4

// Debounce / physical limits
#define MIN_PULSE_US 250   // ignore pulses faster than 250 us (~4000 Hz)
#define SAMPLE_MS 1000     // RPM calculation window

// Smoothing
#define EMA_ALPHA 0.2f     // 0..1, larger = less smoothing

// RPM limits / overspeed
#define MIN_REAL_RPM 100
#define MAX_REAL_RPM 7000
#define OVERSPEED_LIMIT 5100
#define OVERSPEED_RESET 4750

static volatile uint32_t pulse_count = 0;
volatile int last_pulse_time_us = 0;
static volatile uint32_t gpio_pulse_total = 0;

static float rpm_ema = 0.0f;
static bool overspeed_state = false;

// ISR increments pulse_count with debounce
static void IRAM_ATTR tach_isr_handler(void* arg) {
    uint64_t now = esp_timer_get_time();
    uint64_t last = last_pulse_time_us;

    if ((now - last) < MIN_PULSE_US) {
        return; 
    }
    last_pulse_time_us = now;
    pulse_count++;      // safe: 32-bit increment in ISR
    gpio_pulse_total++;
}

void setup_tach_gpio() {
    gpio_config_t io_conf = {
        .intr_type = GPIO_INTR_NEGEDGE, // adjust if your opto pulls HIGH
        .mode = GPIO_MODE_INPUT,
        .pin_bit_mask = (1ULL << TACH_GPIO),
        .pull_up_en = 1,
        .pull_down_en = 0
    };
    gpio_config(&io_conf);
    gpio_install_isr_service(0);
    gpio_isr_handler_add(TACH_GPIO, tach_isr_handler, NULL);
}

 void i2c_Enable_GPIO() {
    gpio_reset_pin(I2C_ENABLE);
    gpio_set_direction(I2C_ENABLE, GPIO_MODE_OUTPUT);
    gpio_set_level(I2C_ENABLE, 1);
}

int read_tach_GPIO(uint32_t last_total, int64_t last_time){
    int64_t now = esp_timer_get_time();
        uint32_t pulses = pulse_count - last_total;
        last_total = pulse_count;

        double elapsed_sec = (now - last_time) / 1e6;
        last_time = now;

        double freq = pulses / (elapsed_sec > 0.0 ? elapsed_sec : 1.0); // pulses per second
        double raw_rpm = (freq * 60.0) / PULSES_PER_REV; // freq*15 for V8

        // clamp noise
        if (raw_rpm < MIN_REAL_RPM) raw_rpm = 0;
        if (raw_rpm > MAX_REAL_RPM) raw_rpm = MAX_REAL_RPM;

        // EMA smoothing
        if (rpm_ema == 0.0f) rpm_ema = raw_rpm;
        else rpm_ema = EMA_ALPHA * raw_rpm + (1.0f - EMA_ALPHA) * rpm_ema;

        rpm_display = (int)(rpm_ema + 0.5f);

        // Overspeed with hysteresis
        if (!overspeed_state && rpm_display >= OVERSPEED_LIMIT) {
            overspeed_state = true;
            ESP_LOGW("TACHO", "OVERSPEED! RPM=%d", rpm_display);
        } else if (overspeed_state && rpm_display <= OVERSPEED_RESET) {
            overspeed_state = false;
            ESP_LOGW("TACHO", "OVERSPEED reset. RPM=%d", rpm_display);
        }

        // Map to gauge % (0..100) using user min/max, clamp
        const int user_min = 0;   // user-settable, e.g., 0 RPM
        const int user_max = 6000; // engine redline for gauge mapping
        int pct = 0; // percentage
        if (rpm_display <= user_min) pct = 0;
        else if (rpm_display >= user_max) pct = 100;
        else pct = (rpm_display - user_min) * 100 / (user_max - user_min);

        int adc_tach_val = tach_value; // uses your existing variable
        // tach_value = rpm_display;
        // Log
        ESP_LOGI("TACHO", "Pulses: %ld, Freq: %.1f Hz, RPM: %d, %%=%d, ADC=%d, GPIO_pulses_total=%ld",
                 pulses, freq, rpm_display, pct, adc_tach_val, gpio_pulse_total);
                 
    return rpm_display;
}

void ads111x_test(void *pvParameters)
{
    
    init_ads1115();
    BLEinit();
    NMEA_init();
    setup_tach_gpio();

    uint32_t lastTotal = 0;
    int64_t lastTime = esp_timer_get_time();

    while (1) {

        readSCL();
        readGND();
        readVCC();
        tach_value = read_tach_GPIO(lastTotal, lastTime);

        ESP_LOGI("MAIN" , "VOLT_SIG = %d, voltage_by_ads = %.2f  ,SWITCH = %d ,tach_value = %d RPM, tilt_trim_value = %d ,oil_press_value = %d PSI ,coolant_value = %d degrees C,warning = %d, fuel_1 = %d %%, fuel_2 = %d %% ,starter= %.2f" ,
            VOLT_SIG, voltage_by_ads, SWITCH, tach_value, tilt_trim_value, oil_press_value, coolant_value, warning, fuel_1, fuel_2, starter_voltage);

        vTaskDelay(pdMS_TO_TICKS(500));
    }
}

void app_main()
{
    i2c_Enable_GPIO();
    ESP_ERROR_CHECK(i2cdev_init());
    xTaskCreatePinnedToCore(ads111x_test, "ads111x_test", 4096, NULL, 5, NULL, APP_CPU_NUM);
}


