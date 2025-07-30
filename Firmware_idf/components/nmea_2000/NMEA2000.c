/*
 * SPDX-FileCopyrightText: 2010-2024 Espressif Systems (Shanghai) CO LTD
 *
 * SPDX-License-Identifier: CC0-1.0
 */

#include <stdio.h>
#include <stdlib.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "freertos/semphr.h"
#include "esp_err.h"
#include "esp_log.h"
#include "driver/twai.h"
#include "NMEA2000.h"
#include "ads1115_readings.h"
#include <string.h>
/* --------------------- Definitions ------------------ */
#define DATA_PERIOD_MS                  50
#define NO_OF_ITERS                     3
#define ITER_DELAY_MS                   1000
#define TX_GPIO_NUM                     GPIO_NUM_17
#define RX_GPIO_NUM                     GPIO_NUM_16
#define EXAMPLE_TAG                     "TWAI Master"

#define ID_MASTER_STOP_CMD              0x0A0
#define ID_MASTER_START_CMD             0x0A1
#define ID_MASTER_PING                  0x0A2
#define ID_SLAVE_STOP_RESP              0x0B0
#define ID_SLAVE_DATA                   0x0B1
#define ID_SLAVE_PING_RESP              0x0B2

static const twai_general_config_t g_config = TWAI_GENERAL_CONFIG_DEFAULT(TX_GPIO_NUM, RX_GPIO_NUM, TWAI_MODE_NO_ACK);
static const twai_timing_config_t t_config = TWAI_TIMING_CONFIG_25KBITS();
static const twai_filter_config_t f_config = TWAI_FILTER_CONFIG_ACCEPT_ALL();

static twai_message_t ping_message = {
    .extd = 0,
    .rtr = 0,
    .ss = 0,
    .self = 0,
    .dlc_non_comp = 0,
    .identifier = ID_MASTER_PING,
    .data_length_code = 0,
};

static twai_message_t start_cmd = {
    .extd = 0,
    .rtr = 0,
    .ss = 0,
    .self = 0,
    .dlc_non_comp = 0,
    .identifier = ID_MASTER_START_CMD,
    .data_length_code = 0,
};

static twai_message_t stop_cmd = {
    .extd = 0,
    .rtr = 0,
    .ss = 0,
    .self = 0,
    .dlc_non_comp = 0,
    .identifier = ID_MASTER_STOP_CMD,
    .data_length_code = 0,
};

// This data message will be modified before sending
static twai_message_t data_message = {
    .extd = 0,
    .rtr = 0,
    .ss = 0,
    .self = 0,
    .dlc_non_comp = 0,
    .identifier = ID_SLAVE_DATA,
    .data_length_code = 4,
    .data = {0},
};

/* -------------------- Master Task --------------------- */

void master_task(void *arg)
{
    while(1){
    for (int iter = 0; iter < NO_OF_ITERS; iter++) {
        ESP_ERROR_CHECK(twai_start());
        ESP_LOGI(EXAMPLE_TAG, "Driver started");
        
        // Step 1: Send Ping to slave
        ESP_LOGI(EXAMPLE_TAG, "Sending ping to slave");
        ESP_ERROR_CHECK(twai_transmit(&ping_message, pdMS_TO_TICKS(1000)));
        
        // Step 2: Wait for ping response from slave
        twai_message_t rx_msg;

        // while (1) {
            //     if (twai_receive(&rx_msg, pdMS_TO_TICKS(1000)) == ESP_OK && rx_msg.identifier == ID_SLAVE_PING_RESP) {
        //         ESP_LOGI(EXAMPLE_TAG, "Received ping response from slave");
        //         break;
        //     }
        // }
        
        // Step 3: Send Start Command
        ESP_LOGI(EXAMPLE_TAG, "Sending start command");
        ESP_ERROR_CHECK(twai_transmit(&start_cmd, pdMS_TO_TICKS(1000)));
        
        // Step 4: Send data for a while
        ESP_LOGI(EXAMPLE_TAG, "Sending data to slave");
        // for (int i = 0; i < 20; i++) {  // Send 20 data packets
        //     uint32_t sensor_data = xTaskGetTickCount();
        //     for (int j = 0; j < 4; j++) {
            //         data_message.data[j] = (sensor_data >> (j * 8)) & 0xFF;
            //     }
            //     ESP_ERROR_CHECK(twai_transmit(&data_message, pdMS_TO_TICKS(1000)));
            //     ESP_LOGI(EXAMPLE_TAG, "Sent data: %" PRIu32, sensor_data);
            //     vTaskDelay(pdMS_TO_TICKS(DATA_PERIOD_MS));
            // }
            #define JSON_BUFFER_SIZE 256
        char json_buffer[JSON_BUFFER_SIZE];
        get_sensor_data_json(json_buffer, sizeof(json_buffer));
        ESP_LOGI(EXAMPLE_TAG, "Generated JSON: %s", json_buffer);
        
        size_t json_len = strlen(json_buffer);
        ESP_LOGI(EXAMPLE_TAG, "Size of JSON: %d", json_len);
        
        size_t offset = 0;

        while (offset < json_len) {
            
            size_t chunk_len = (json_len - offset > 8) ? 8 : (json_len - offset);
            ESP_LOGI(EXAMPLE_TAG, "chunk size is : %d", chunk_len);
            twai_message_t json_msg = {
                .extd = 0,
                .rtr = 0,
                .ss = 0,
                .self = 0,
                .identifier = ID_SLAVE_DATA,  // or define a new one
                .data_length_code = chunk_len,
            };
            
            memcpy(json_msg.data, json_buffer + offset, chunk_len);
            
            // ESP_ERROR_CHECK(twai_transmit(&json_msg, pdMS_TO_TICKS(1000)));
            esp_err_t err = twai_transmit(&json_msg, pdMS_TO_TICKS(1000));
            if (err == ESP_OK) {
                ESP_LOGI(EXAMPLE_TAG, "Sent JSON chunk: len=%d, and data= %.*s", (int)chunk_len, chunk_len, json_msg.data);
                vTaskDelay(pdMS_TO_TICKS(100));
            }
            else{
                ESP_LOGE(EXAMPLE_TAG, "Failed to send JSON chunk at offset %d, err=0x%x", offset, err);
                break;
            }
            
            offset += chunk_len;

            vTaskDelay(pdMS_TO_TICKS(100));
        }
        
        
        // Step 5: Send Stop Command
        ESP_LOGI(EXAMPLE_TAG, "Sending stop command");
        ESP_ERROR_CHECK(twai_transmit(&stop_cmd, pdMS_TO_TICKS(1000)));

        // Step 6: Wait for stop response from slave
        // while (1) {
        //     if (twai_receive(&rx_msg, pdMS_TO_TICKS(1000)) == ESP_OK && rx_msg.identifier == ID_SLAVE_STOP_RESP) {
        //         ESP_LOGI(EXAMPLE_TAG, "Received stop response from slave");
        //         break;
        //     }
        // }
        
        ESP_ERROR_CHECK(twai_stop());
        ESP_LOGI(EXAMPLE_TAG, "Driver stopped");
        vTaskDelay(pdMS_TO_TICKS(ITER_DELAY_MS));
    }

    // Clean up
}
    vTaskDelete(NULL);
}

/* -------------------- app_main --------------------- */

void NMEA_init(void)
{
    // Wait a bit for slave to boot
    for (int i = 3; i > 0; i--) {
        printf("Master starting in %d\n", i);
        vTaskDelay(pdMS_TO_TICKS(1000));
    }

    ESP_ERROR_CHECK(twai_driver_install(&g_config, &t_config, &f_config));
    ESP_LOGI(EXAMPLE_TAG, "TWAI driver installed");
    
    xTaskCreatePinnedToCore(master_task, "master_task", 4096, NULL, 10, NULL, tskNO_AFFINITY);
    
}
