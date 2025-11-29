#include "basshero.h"
#include "esp_log.h"

static const char* TAG = "BassHero";

constexpr int I2C_MASTER_SCL_IO = 22;
constexpr int I2C_MASTER_SDA_IO = 21;
constexpr i2c_port_t I2C_MASTER_NUM = I2C_NUM_0;
constexpr int I2C_MASTER_FREQ_HZ = 400000;

BassHero::BassHero() : sd(19, 23, 18, 5) {
    if (!sd.mount("/sdcard")) {
        printf("Mount failed\n");
        return;
    }
}

BassHero::~BassHero() {
    sd.unmount();
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
        "E", "A", "D", "G",
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

    // sd card test
    self->sd.writeFile("/sdcard/test.txt", "Hello from C++ class!\n");
    self->sd.appendFile("/sdcard/test.txt", "Appending line...\n");

    self->sd.writeFile("/sdcard/tabs/newTab.bh1", "Example Tabs\n");

    const std::vector<std::string> files = self->sd.listDirectory("/sdcard");
    ESP_LOGI(TAG, "ls sdcard");
    for (const auto& f : files) {
        ESP_LOGI(TAG, "%s", f.c_str());
    }

    const std::vector<std::string> files2 = self->sd.listDirectory("/sdcard/tabs");
    ESP_LOGI(TAG, "ls sdcard/tabs");
    for (const auto& f : files2) {
        ESP_LOGI(TAG, "%s", f.c_str());
    }
    
    const std::string contents = self->sd.readFile("/sdcard/tabs/newTab.bh1");
    ESP_LOGI(TAG, "Reading file: /sdcard/test.txt:\n%s", contents.c_str());

    while (true) {
        self->updateDisplay();
        self->scrollIndex++;
        vTaskDelay(pdMS_TO_TICKS(msDelay));
    }
}

void BassHero::startGame() {
    xTaskCreatePinnedToCore(gameTask, "basshero_task", 4096, this, 5, nullptr, 1);
}
