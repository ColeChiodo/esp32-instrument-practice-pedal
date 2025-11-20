#pragma once
#include "driver/i2c.h"
#include "esp_err.h"
#include <stdint.h>

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64

esp_err_t lcd_init(i2c_port_t i2c_num, uint8_t address);
esp_err_t lcd_clear_screen(void);
esp_err_t lcd_display_text(const char* text, uint8_t line);
esp_err_t lcd_draw_pixel(uint8_t x, uint8_t y, bool color);
esp_err_t lcd_update(void);
