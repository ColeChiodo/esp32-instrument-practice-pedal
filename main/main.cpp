#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/i2c_master.h"
#include "esp_log.h"

#include "lcd_driver.h"

#define I2C_MASTER_SCL_IO 22
#define I2C_MASTER_SDA_IO 21
#define I2C_MASTER_NUM    I2C_NUM_0
#define I2C_MASTER_FREQ_HZ 400000

extern "C" void app_main(void) {
    i2c_config_t conf = {};
    conf.mode = I2C_MODE_MASTER;
    conf.sda_io_num = 21;
    conf.scl_io_num = 22;
    conf.sda_pullup_en = GPIO_PULLUP_ENABLE;
    conf.scl_pullup_en = GPIO_PULLUP_ENABLE;
    conf.master.clk_speed = 100000;
    i2c_param_config(I2C_NUM_0, &conf);
    i2c_driver_install(I2C_NUM_0, I2C_MODE_MASTER, 0, 0, 0);

    lcd_init(I2C_NUM_0, 0x3C);
    lcd_clear_screen();
    lcd_display_text("Creep - Radiohead", 0);
    lcd_display_text("G|----------------------------|----------------------------|", 2);
    lcd_display_text("D|----------------------------|----------------------------|", 3);
    lcd_display_text("A|----------------------------|--2----2-2----2----2-2------|", 4);
    lcd_display_text("E|--3---33-3---33---33-3---3--|------2------2----2------2--|", 5);
    lcd_display_text("colechiodo.cc", 7);

    // Draw some pixels
    lcd_update();

    while (true) {
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}