#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"
#include "esp_log.h"

// Onboard LED pin (ESP32 DevKitC typically GPIO 2)
#define LED_PIN GPIO_NUM_2

extern "C" void app_main(void)
{
    // Configure LED pin
    gpio_reset_pin(LED_PIN);
    gpio_set_direction(LED_PIN, GPIO_MODE_OUTPUT);

    ESP_LOGI("BassTabHero", "Hello BassTab! Starting main loop...");

    while (true) {
        // Blink LED
        gpio_set_level(LED_PIN, 1);
        vTaskDelay(pdMS_TO_TICKS(500));  // 500 ms ON
        gpio_set_level(LED_PIN, 0);
        vTaskDelay(pdMS_TO_TICKS(500));  // 500 ms OFF
    }
}
