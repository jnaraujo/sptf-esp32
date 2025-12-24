#include "DisplayManager.hpp"
#include "StringUtils.hpp"

void DisplayManager::begin() {
    if (!this->display.begin(SSD1306_SWITCHCAPVCC, Config::OLED_ADDR)) {
        return;
    }
    this->display.clearDisplay();
    
    this->u8g2.begin(display);
    this->u8g2.setFontMode(1);
    this->u8g2.setFontDirection(0);
    this->u8g2.setForegroundColor(SSD1306_WHITE);
    
    this->display.display();
}

void DisplayManager::render(const PlaybackState& state) {
    this->display.clearDisplay();

    this->u8g2.setFont(u8g2_font_profont10_tf);
    this->u8g2.setCursor(0, Layout::HEADER_Y);
    String header = StringUtils::formatString(
        state.artist,
        Layout::ARTIST_NUM_LINES,
        Layout::MAX_CHARS_ARTIST
    ) + " - " + state.album;
    this->u8g2.print(StringUtils::formatString(
        header,
        Layout::HEADER_NUM_LINES,
        Layout::MAX_CHARS_HEADER
    ));

    this->u8g2.setFont(u8g2_font_profont17_tf);
    this->u8g2.setCursor(0, Layout::TITLE_Y);
    this->u8g2.print(StringUtils::wordWrap(StringUtils::formatString(
        state.title,
        Layout::TITLE_NUM_LINES,
        Layout::MAX_CHARS_TITLE
    ), Layout::MAX_CHARS_TITLE));

    drawPlayerStatus(state.isPlaying);
    drawProgressBar(state.progress_ms, state.duration_ms);

    this->display.display();
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
    
    this->u8g2.setFont(u8g2_font_profont10_tf);
    this->u8g2.setCursor(0, Layout::STATUS_Y);
    this->u8g2.print(StringUtils::centerString(face, Layout::STATUS_WIDTH_CHARS).c_str());
}

void DisplayManager::drawProgressBar(int progress_ms, int duration_ms) {
    if (duration_ms > 0) {
        int width = map(progress_ms, 0, duration_ms, 0, Config::DISPLAY_WIDTH);
        this->display.fillRect(0, Config::DISPLAY_HEIGHT-1, width, 1, SSD1306_WHITE);
    }
}