/*
 * This example code is in the Public Domain (or CC0 licensed, at your option.)
 *
 * Unless required by applicable law or agreed to in writing, this
 * software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
 * CONDITIONS OF ANY KIND, either express or implied.
 *
 * This example demonstrates how to use RMT to control a single SK68XXMINI-HS LED
 * strip connected to GPIO38 on ESP32-S3-DevKitC-1 V1.1.
 * 
 * Make sure the board is V1.1, as the GPIO for the LED is GPIO48 on V1.0.
 */

 #include <stdio.h>
 #include "freertos/FreeRTOS.h"
 #include "freertos/task.h"
 #include "driver/rmt_tx.h"
 #include "esp_log.h"
 
 #define LED_STRIP_GPIO 38  // GPIO where the SK68XXMINI-HS LED is connected on ESP32-S3-DevKitC-1 V1.1
 #define LED_NUMBERS 1      // Number of LEDs in the strip
 
 static const char *TAG = "SK68XXMINI-HS";
 
 rmt_channel_handle_t led_chan = NULL;
 rmt_encoder_handle_t led_encoder = NULL;
 
 void configure_rmt() {
     ESP_LOGI(TAG, "Configuring RMT for SK68XXMINI-HS");
 
     rmt_tx_channel_config_t tx_chan_config = {
         .gpio_num = LED_STRIP_GPIO,  // Set to GPIO38
         .clk_src = RMT_CLK_SRC_DEFAULT,
         .resolution_hz = 20 * 1000 * 1000,  // 20 MHz = 0.05µs per tick
         .mem_block_symbols = 64,
         .trans_queue_depth = 4,
     };
     ESP_ERROR_CHECK(rmt_new_tx_channel(&tx_chan_config, &led_chan));
 
     /* Adjusted timing values for SK68XX
      * Total bit time = 0.35µs + 1µs = 1.35µs
      * This is adjusted for the SK68XXMINI-HS LED however 
      * standard WS2812 timings should work as well, they worked in seting with board V1.0
      */
     rmt_bytes_encoder_config_t bytes_encoder_config = {
         .bit0 = { .level0 = 1, .duration0 = 7,  .level1 = 0, .duration1 = 20 },  // "0" bit: High 0.35µs, Low 1µs
         .bit1 = { .level0 = 1, .duration0 = 20,  .level1 = 0, .duration1 = 7 },  // "1" bit: High 1µs, Low 0.35µs
         .flags.msb_first = 1
     };
     ESP_ERROR_CHECK(rmt_new_bytes_encoder(&bytes_encoder_config, &led_encoder));
 
     ESP_ERROR_CHECK(rmt_enable(led_chan));
 }
 
 
 
 // Function to send color data to SK68XXMINI-HS
 void send_led_color(uint8_t red, uint8_t green, uint8_t blue) {
     uint8_t led_data[3] = { green, red, blue };
     rmt_transmit_config_t tx_config = { .loop_count = 0 };
     ESP_ERROR_CHECK(rmt_transmit(led_chan, led_encoder, led_data, sizeof(led_data), &tx_config));
     ESP_ERROR_CHECK(rmt_tx_wait_all_done(led_chan, pdMS_TO_TICKS(100)));
 }
 
 void app_main() {
     configure_rmt();
 
     while (1) {
         ESP_LOGI(TAG, "Setting LED to RED");
         send_led_color(255, 0, 0);  // Red
         vTaskDelay(pdMS_TO_TICKS(1000));
 
         ESP_LOGI(TAG, "Setting LED to GREEN");
         send_led_color(0, 255, 0);  // Green
         vTaskDelay(pdMS_TO_TICKS(1000));
 
         ESP_LOGI(TAG, "Setting LED to BLUE");
         send_led_color(0, 0, 255);  // Blue
         vTaskDelay(pdMS_TO_TICKS(1000));
 
         ESP_LOGI(TAG, "Turning LED OFF");
         send_led_color(0, 0, 0);  // Off
         vTaskDelay(pdMS_TO_TICKS(1000));
     }
 }
 