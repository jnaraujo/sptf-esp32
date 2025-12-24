#pragma once

#include <array>
#include <cstdint>

namespace Config {
// --- Pins ---
constexpr uint8_t PIN_OLED_SCL = 5;
constexpr uint8_t PIN_OLED_SDA = 4;

// --- Display ---
constexpr uint8_t OLED_ADDR = 0x3C;
constexpr int DISPLAY_WIDTH = 128;
constexpr int DISPLAY_HEIGHT = 64;

// --- Timing ---
constexpr uint32_t REQUEST_POOL_EXEC_INTERVAL_MS = 250;
constexpr uint32_t FETCH_SPOTIFY_STATE_INTERVAL_MS = 1000;
constexpr uint32_t REFRESH_TOKEN_INTERVAL_SECS = 3600;
constexpr uint32_t DEBOUNCE_DELAY_MS = 50;
constexpr uint32_t WIFI_RECONNECT_INTERVAL_MS = 500;

// --- Button Map ---
enum ButtonType { BTN_UP = 0, BTN_DOWN, BTN_LEFT, BTN_RIGHT, BTN_CONFIRM };

constexpr uint8_t BTN_COUNT = 5;

constexpr std::array<uint8_t, BTN_COUNT> BTN_PINS = {
	10,	 // UP
	11,	 // DOWN
	13,	 // LEFT
	12,	 // RIGHT
	9	 // CONFIRM
};
}  // namespace Config