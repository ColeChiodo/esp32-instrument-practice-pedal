#include "basshero.h"
#include "http_server.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

extern "C" void app_main(void) {
    static BassHero bassHero;
    bassHero.startGame();

    start_http_server_task();

    while (true) {
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}