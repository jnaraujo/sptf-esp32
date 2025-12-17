#pragma once

#include <Adafruit_SSD1306.h>
#include <U8g2_for_Adafruit_GFX.h>
#include "Config.hpp"
#include "spotify.h"

class DisplayManager {
public:
    DisplayManager();
    void begin();
    void render(const PlaybackState& state);

private:
    Adafruit_SSD1306 display;
    U8G2_FOR_ADAFRUIT_GFX u8g2;
    
    // Internal helpers
    void drawPlayerStatus(bool isPlaying);
    void drawProgressBar(int progress_ms, int duration_ms);
    bool shouldBlink = false;
    uint32_t lastBlinkTime = 0;
};