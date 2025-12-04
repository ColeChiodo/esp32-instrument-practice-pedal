#pragma once

#include <array>
#include <vector>
#include <string>
#include <cstring>

#include "oled_driver.h"
#include "sd_driver.h"
#include "button.h"

#include "driver/i2c_master.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

class BassHero {
    public:
        BassHero();
        ~BassHero();
        void startGame();

    private:
        static constexpr int DISPLAY_WIDTH_CHARS = 20;
        static constexpr int NUM_STRINGS = 4;
        size_t tuneWidth = 0;

        SDCard sd;

        struct TabBar {
            std::array<std::string, NUM_STRINGS> lines;
        };

        struct Tab {
            Metadata meta;
			int bpm{};
            int beatsPerBar{};
            int tabCharsPerBar{};
            std::vector<std::string> tuning;
            std::vector<TabBar> bars;
        };

        Tab currentTab;
        unsigned int scrollIndex = 0;
        char lineBuffer[DISPLAY_WIDTH_CHARS + 2]{};

        void initializeOLED();
        void loadSong(std::string file, auto* self);
        int getDelayMs() const;
        void updateDisplay();
		void selectInstrument(auto* self, std::string file);
		void playTab(auto* self, std::string file, std::string instrument);
        static void gameTask(void* param);
};
