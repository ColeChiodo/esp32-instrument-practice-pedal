#include "http_server.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

static void http_server_task(void* param) {
    // Initialize WiFi and HTTP server here
    // Handle /upload POST requests
    while (true) {
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}

void start_http_server_task() {
    xTaskCreatePinnedToCore(http_server_task, "http_server_task", 8192, nullptr, 5, nullptr, 1);
}
