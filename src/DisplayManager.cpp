#include "DisplayManager.hpp"
#include "StringUtils.hpp"

DisplayManager::DisplayManager() 
    : display(DISPLAY_WIDTH, DISPLAY_HEIGHT, &Wire, -1) {}

void DisplayManager::begin() {
    if (!display.begin(SSD1306_SWITCHCAPVCC, OLED_ADDR)) {
        return;
    }
    display.clearDisplay();
    
    u8g2.begin(display);
    u8g2.setFontMode(1);
    u8g2.setFontDirection(0);
    u8g2.setForegroundColor(SSD1306_WHITE);
    
    display.display();
}

void DisplayManager::render(const PlaybackState& state) {
    display.clearDisplay();

    u8g2.setFont(u8g2_font_profont10_tf);
    u8g2.setCursor(0, 10);
    String header = StringUtils::formatString(state.artist, 1, 14) + " - " + state.album;
    u8g2.print(StringUtils::formatString(header, 1, 24));

    u8g2.setFont(u8g2_font_profont17_tf);
    u8g2.setCursor(0, 29);
    u8g2.print(StringUtils::wordWrap(StringUtils::formatString(state.title, 2, 14), 14));

    drawPlayerStatus(state.isPlaying);
    drawProgressBar(state.progress_ms, state.duration_ms);

    display.display();
}

void DisplayManager::drawPlayerStatus(bool isPlaying) {
    if (millis() - lastBlinkTime > 500) {
        shouldBlink = !shouldBlink;
        lastBlinkTime = millis();
    }

    String face = isPlaying ? "(^-^)/" : "(-.-) zZ";
    if (shouldBlink) {
        face = isPlaying ? "(^O^)_" : "(-.-) Zz";
    }
    
    u8g2.setFont(u8g2_font_profont10_tf);
    u8g2.setCursor(0, 58);
    u8g2.print(StringUtils::centerString(face, 24).c_str());
}

void DisplayManager::drawProgressBar(int progress_ms, int duration_ms) {
    if (duration_ms > 0) {
        int width = map(progress_ms, 0, duration_ms, 0, DISPLAY_WIDTH);
        display.fillRect(0, DISPLAY_HEIGHT-1, width, 1, SSD1306_WHITE);
    }
}