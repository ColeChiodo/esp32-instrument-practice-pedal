#pragma once

#include <array>
#include <vector>
#include <string>

class BassHero {
    public:
        BassHero();
        void startGame();

    private:
        static constexpr int DISPLAY_WIDTH_CHARS = 20;
        static constexpr int NUM_STRINGS = 4;
        size_t tuneWidth = 0;

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
