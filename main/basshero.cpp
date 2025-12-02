#include "basshero.h"
#include "esp_log.h"
#include "oled_driver.h"

static const char* TAG = "BassHero";

constexpr gpio_num_t BUTTON_UP = GPIO_NUM_12;
constexpr gpio_num_t BUTTON_DOWN = GPIO_NUM_13;
constexpr gpio_num_t BUTTON_SELECT = GPIO_NUM_14;

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
    // self->loadExampleSong();

    // // Display header
    // oled_display_text(self->currentSong.title.c_str(), 0);
    // oled_display_text("colechiodo.cc", 7);
    // oled_update();

    // int msDelay = self->getDelayMs();

    // for (int i = 0; i < self->currentSong.tuning.size(); ++i) {
    //     self->tuneWidth = std::max(self->tuneWidth, self->currentSong.tuning[i].size());
    // }

    // sd card test
	self->sd.writeFile("/sdcard/tabs/test0.bh1", "Test");
	self->sd.writeFile("/sdcard/tabs/test1.bh1", "Test");
	self->sd.writeFile("/sdcard/tabs/test2.bh1", "Test");
	self->sd.writeFile("/sdcard/tabs/test3.bh1", "Test");
	self->sd.writeFile("/sdcard/tabs/test4.bh1", "Test");
	self->sd.writeFile("/sdcard/tabs/test5.bh1", "Test");
	self->sd.writeFile("/sdcard/tabs/test6.bh1", "Test");
	self->sd.writeFile("/sdcard/tabs/test7.bh1", "Test");
	self->sd.writeFile("/sdcard/tabs/test8.bh1", "Test");
	self->sd.writeFile("/sdcard/tabs/test9.bh1", "Test");
	self->sd.writeFile("/sdcard/tabs/test10.bh1", "Test");
	self->sd.writeFile("/sdcard/tabs/test11.bh1", "Test");
	self->sd.writeFile("/sdcard/tabs/test12.bh1", "Test");
	self->sd.writeFile("/sdcard/tabs/test13.bh1", "Test");
	self->sd.writeFile("/sdcard/tabs/test14.bh1", "Test");
	self->sd.writeFile("/sdcard/tabs/test15.bh1", "Test");
	self->sd.writeFile("/sdcard/tabs/test16.bh1", "Test");
	self->sd.writeFile("/sdcard/tabs/test17.bh1", "Test");
	self->sd.writeFile("/sdcard/tabs/test18.bh1", "Test");
	self->sd.writeFile("/sdcard/tabs/test19.bh1", "Test");
	
    const std::vector<std::string> files = self->sd.listDirectory("/sdcard/tabs");
	ESP_LOGI(TAG, "Tab files found: %d", files.size());
	ESP_LOGI(TAG, "ls sdcard/tabs");
    for (const auto& f : files) {
        ESP_LOGI(TAG, "%s", f.c_str());
    }

    int scrollIndex = 0;
	int selectIndex = 0;
	std::string selectPrefix = "> ";
	
	Button btnUp(BUTTON_UP);
	Button btnDown(BUTTON_DOWN);
	Button btnSelect(BUTTON_SELECT);

    while (true) {
        // self->updateDisplay();
        // self->scrollIndex++;
		if (btnUp.isPressed()) {
			if (selectIndex != 0) selectIndex--;
			else if (scrollIndex != 0) {
				scrollIndex--;
				oled_clear_screen();
			}
		}
		if (btnDown.isPressed()) {
			if (selectIndex != files.size() - 1 && selectIndex < 7) selectIndex++;
			else if (selectIndex == 7 && scrollIndex + 7 < files.size() - 1) {
				scrollIndex++;
				oled_clear_screen();
			}
		}
		if (btnSelect.isPressed()) {
			ESP_LOGI(TAG, "Selected %s", files[scrollIndex + selectIndex].c_str());
		}

		for (int i = 0; i < 8; i++) {
			if (scrollIndex + i >= files.size()) break;
			if (selectIndex == i) {
				oled_display_text((selectPrefix + files[scrollIndex + i]).c_str(), i);
			} else {
				oled_display_text(files[scrollIndex + i].c_str(), i);
			}
		}
		oled_update();

        // vTaskDelay(pdMS_TO_TICKS(100));
    }
}

void BassHero::startGame() {
    xTaskCreatePinnedToCore(gameTask, "basshero_task", 4096, this, 5, nullptr, 1);
}
