#include "basshero.h"
#include "oled_driver.h"
#include "driver/i2c_master.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include <cstring>
#include <vector>

constexpr int I2C_MASTER_SCL_IO = 22;
constexpr int I2C_MASTER_SDA_IO = 21;
constexpr i2c_port_t I2C_MASTER_NUM = I2C_NUM_0;
constexpr int I2C_MASTER_FREQ_HZ = 400000;

BassHero::BassHero() {
    // Constructor can preload song or initialize members if needed
}

void BassHero::initializeOLED() {
    i2c_config_t conf = {};
    conf.mode = I2C_MODE_MASTER;
    conf.sda_io_num = I2C_MASTER_SDA_IO;
    conf.scl_io_num = I2C_MASTER_SCL_IO;
    conf.sda_pullup_en = GPIO_PULLUP_ENABLE;
    conf.scl_pullup_en = GPIO_PULLUP_ENABLE;
    conf.master.clk_speed = I2C_MASTER_FREQ_HZ;

    i2c_param_config(I2C_MASTER_NUM, &conf);
    i2c_driver_install(I2C_MASTER_NUM, I2C_MODE_MASTER, 0, 0, 0);

    oled_init(I2C_MASTER_NUM, 0x3C);
    oled_clear_screen();
}

void BassHero::loadExampleSong() {
    currentSong.title = "Creep - Radiohead";
    currentSong.author = "Radiohead";
    currentSong.bpm = 93;
    currentSong.beatsPerBar = 4;
    currentSong.tabCharsPerBar = 28;

    currentSong.tuning = {
        "Eb", "A", "D", "G#",
    };

    currentSong.bars = {
        { {"|----------------------------|",
           "|--------6-------------------|",
           "|--2----2-2----2----2-2------|",
           "|--3---33-3---33---33-3---3--|"} },
        { {"|-----------1----------------|",
           "|-----------------3---------|",
           "|----------------------------|",
           "|--3---33-3---33---33-3---3--|"} }
    };
}

int BassHero::getDelayMs() const {
    float secondsPerBeat = 60.0f / currentSong.bpm;
    float secondsPerBar = secondsPerBeat * currentSong.beatsPerBar;
    float secondsPerChar = secondsPerBar / currentSong.tabCharsPerBar;
    return static_cast<int>(secondsPerChar * 1000.0f);
}

void BassHero::updateDisplay() {
    for (int i = 0; i < NUM_STRINGS; ++i) {
        const auto& line = currentSong.bars[0].lines[i];
        int len = line.size();
        int pos = scrollIndex % len;

        const std::string& tune = currentSong.tuning[NUM_STRINGS - 1 - i];

        // Write tuning and pad to tuneWidth
        for (size_t k = 0; k < tuneWidth; ++k) {
            if (k < tune.size()) {
                lineBuffer[k] = tune[k];
            } else {
                lineBuffer[k] = ' ';
            }
        }

        // Copy tab characters after tuning
        for (int j = 0; j < DISPLAY_WIDTH_CHARS; ++j) {
            lineBuffer[j + tuneWidth] = line[(pos + j) % len];
        }

        // Null terminate
        lineBuffer[DISPLAY_WIDTH_CHARS + tuneWidth] = '\0';

        // Display
        oled_display_text(lineBuffer, i + 2);
    }

    oled_update();
}

void BassHero::gameTask(void* param) {
    auto* self = static_cast<BassHero*>(param);
    self->initializeOLED();
    self->loadExampleSong();

    // Display header
    oled_display_text(self->currentSong.title.c_str(), 0);
    oled_display_text("colechiodo.cc", 7);
    oled_update();

    int msDelay = self->getDelayMs();

    for (int i = 0; i < self->currentSong.tuning.size(); ++i) {
        self->tuneWidth = std::max(self->tuneWidth, self->currentSong.tuning[i].size());
    }

    while (true) {
        self->updateDisplay();
        self->scrollIndex++;
        vTaskDelay(pdMS_TO_TICKS(msDelay));
    }
}

void BassHero::startGame() {
    xTaskCreatePinnedToCore(gameTask, "basshero_task", 4096, this, 5, nullptr, 1);
}
