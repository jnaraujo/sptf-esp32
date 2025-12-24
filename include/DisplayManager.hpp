#pragma once

#include <Adafruit_SSD1306.h>
#include <U8g2_for_Adafruit_GFX.h>
#include "Config.hpp"
#include "spotify.h"

class DisplayManager {
private:
    Adafruit_SSD1306 display {
        Config::DISPLAY_WIDTH, 
        Config::DISPLAY_HEIGHT, 
        &Wire, 
        -1
    };
    U8G2_FOR_ADAFRUIT_GFX u8g2;

    struct Layout {
        static constexpr int HEADER_Y = 10;
        static constexpr int TITLE_Y  = 29;
        static constexpr int STATUS_Y = 58;

        static constexpr int MAX_CHARS_ARTIST = 14;
        static constexpr int MAX_CHARS_HEADER = 24;
        static constexpr int MAX_CHARS_TITLE  = 14;
        static constexpr int STATUS_WIDTH_CHARS = 24;
        
        static constexpr int ARTIST_NUM_LINES = 1; 
        static constexpr int HEADER_NUM_LINES = 1; 
        static constexpr int TITLE_NUM_LINES  = 2;
    };
    
    // Internal helpers
    void drawPlayerStatus(bool isPlaying);
    void drawProgressBar(int progress_ms, int duration_ms);
    bool shouldBlink = false;
    uint32_t lastBlinkTime = 0;
public:
    DisplayManager() = default;
    void begin();
    void render(const PlaybackState& state);
};