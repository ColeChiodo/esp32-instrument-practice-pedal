#include "lcd_driver.h"
#include "esp_log.h"
#include "string.h"

static const char* TAG = "LCD";
static uint8_t lcd_address = 0x3C;
static i2c_port_t i2c_num = I2C_NUM_0;

// Simple framebuffer: 1 bit per pixel
static uint8_t buffer[SCREEN_WIDTH * SCREEN_HEIGHT / 8];

// Forward declarations
static esp_err_t lcd_write_cmd(uint8_t cmd);
static esp_err_t lcd_write_data(uint8_t* data, size_t len);

// Initialize SSD1306
esp_err_t lcd_init(i2c_port_t port, uint8_t address) {
    i2c_num = port;
    lcd_address = address;
    memset(buffer, 0, sizeof(buffer));

    ESP_LOGI(TAG, "Initializing SSD1306 at 0x%X", address);

    // SSD1306 init sequence
    lcd_write_cmd(0xAE); // display off
    lcd_write_cmd(0x20); // memory mode
    lcd_write_cmd(0x00); // horizontal
    lcd_write_cmd(0xB0); // page start
    lcd_write_cmd(0xC8); // COM scan dec
    lcd_write_cmd(0x00); // low col
    lcd_write_cmd(0x10); // high col
    lcd_write_cmd(0x40); // start line
    lcd_write_cmd(0x81); // contrast
    lcd_write_cmd(0xFF);
    lcd_write_cmd(0xA1); // segment remap
    lcd_write_cmd(0xA6); // normal display
    lcd_write_cmd(0xA8); // multiplex
    lcd_write_cmd(0x3F);
    lcd_write_cmd(0xD3); // display offset
    lcd_write_cmd(0x00);
    lcd_write_cmd(0xD5); // display clock divide
    lcd_write_cmd(0xF0);
    lcd_write_cmd(0xD9); // pre-charge
    lcd_write_cmd(0x22);
    lcd_write_cmd(0xDA); // com pins
    lcd_write_cmd(0x12);
    lcd_write_cmd(0xDB); // vcom detect
    lcd_write_cmd(0x20);
    lcd_write_cmd(0x8D); // charge pump
    lcd_write_cmd(0x14);
    lcd_write_cmd(0xAF); // display on

    ESP_LOGI(TAG, "SSD1306 initialized");
    return lcd_update();
}

// Clear framebuffer
esp_err_t lcd_clear_screen(void) {
    memset(buffer, 0, sizeof(buffer));
    return lcd_update();
}

// Draw text (very simple: only 5x7 font, ASCII 32-127)
#include "fonts.h" // You can include a minimal 5x7 font array
esp_err_t lcd_display_text(const char* text, uint8_t line) {
    if (line >= 8) return ESP_ERR_INVALID_ARG;
    uint8_t x = 0;
    uint8_t y = line * 8;
    while (*text && x < SCREEN_WIDTH - 6) {
        char c = *text++;
        if (c < 32 || c > 127) c = '?';
        for (int i = 0; i < 5; i++) {
            uint8_t col = font5x7[c - 32][i];
            for (int j = 0; j < 8; j++) {
                bool pixel = col & (1 << j);
                lcd_draw_pixel(x, y + j, pixel);
            }
            x++;
        }
        x++; // spacing
    }
    return lcd_update();
}

// Draw single pixel in framebuffer
esp_err_t lcd_draw_pixel(uint8_t x, uint8_t y, bool color) {
    if (x >= SCREEN_WIDTH || y >= SCREEN_HEIGHT) return ESP_ERR_INVALID_ARG;
    uint16_t byte_index = x + (y / 8) * SCREEN_WIDTH;
    if (color)
        buffer[byte_index] |= 1 << (y % 8);
    else
        buffer[byte_index] &= ~(1 << (y % 8));
    return ESP_OK;
}

// Send framebuffer to OLED
esp_err_t lcd_update(void) {
    for (uint8_t page = 0; page < 8; page++) {
        lcd_write_cmd(0xB0 + page);
        lcd_write_cmd(0x00);
        lcd_write_cmd(0x10);

        lcd_write_data(&buffer[page * SCREEN_WIDTH], SCREEN_WIDTH);
    }
    return ESP_OK;
}

// --- Low level I2C commands ---
static esp_err_t lcd_write_cmd(uint8_t cmd) {
    i2c_cmd_handle_t handle = i2c_cmd_link_create();
    i2c_master_start(handle);
    i2c_master_write_byte(handle, (lcd_address << 1) | I2C_MASTER_WRITE, true);
    i2c_master_write_byte(handle, 0x00, true); // Co = 0, D/C# = 0 -> command
    i2c_master_write_byte(handle, cmd, true);
    i2c_master_stop(handle);
    esp_err_t ret = i2c_master_cmd_begin(i2c_num, handle, pdMS_TO_TICKS(1000));
    i2c_cmd_link_delete(handle);
    return ret;
}

static esp_err_t lcd_write_data(uint8_t* data, size_t len) {
    i2c_cmd_handle_t handle = i2c_cmd_link_create();
    i2c_master_start(handle);
    i2c_master_write_byte(handle, (lcd_address << 1) | I2C_MASTER_WRITE, true);
    i2c_master_write_byte(handle, 0x40, true); // Co = 0, D/C# = 1 -> data
    i2c_master_write(handle, data, len, true);
    i2c_master_stop(handle);
    esp_err_t ret = i2c_master_cmd_begin(i2c_num, handle, pdMS_TO_TICKS(1000));
    i2c_cmd_link_delete(handle);
    return ret;
}
