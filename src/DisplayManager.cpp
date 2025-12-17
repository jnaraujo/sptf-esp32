#include "DisplayManager.hpp"

#include "DisplayManager.hpp"
#include "StringUtils.hpp"

DisplayManager::DisplayManager() 
    : display(DISPLAY_WIDTH, DISPLAY_HEIGHT, &Wire, -1) {}

void DisplayManager::begin() {
    display.begin(SSD1306_SWITCHCAPVCC, OLED_ADDR);
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

    drawBlinkFace(state.isPlaying);

    display.display();
}

void DisplayManager::drawBlinkFace(bool isPlaying) {
    if (millis() - lastBlinkTime > 500) {
        shouldBlink = !shouldBlink;
        lastBlinkTime = millis();
    }

    String face = isPlaying ? "(^-^)/" : "(-.-) zZ";
    String pad = "=-=-=-=";

    if (shouldBlink) {
        if (isPlaying) {
            face = "(^O^)_";
            pad = "-=-=-=-";
        } else {
            face = "(-.-) Zz";
        }
    }
    
    u8g2.setFont(u8g2_font_profont10_tf);
    u8g2.setCursor(0, 60);
    u8g2.printf("%s %s %s", pad.c_str(), StringUtils::centerString(face, 10).c_str(), pad.c_str());
}