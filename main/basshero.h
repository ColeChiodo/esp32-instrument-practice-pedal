#pragma once

#include <array>
#include <vector>
#include <string>
#include <cstring>

#include "oled_driver.h"
#include "sd_driver.h"

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

        struct Song {
            std::string title;
            std::string author;
            int bpm{};
            int beatsPerBar{};
            int tabCharsPerBar{};
            std::vector<std::string> tuning;
            std::vector<TabBar> bars;
        };

        Song currentSong;
        unsigned int scrollIndex = 0;
        char lineBuffer[DISPLAY_WIDTH_CHARS + 2]{};

        void initializeOLED();
        void loadExampleSong();
        int getDelayMs() const;
        void updateDisplay();
        static void gameTask(void* param);
};
