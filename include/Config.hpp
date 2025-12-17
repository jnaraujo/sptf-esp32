#pragma once

// --- Pins ---
#define OLED_SCL 5
#define OLED_SDA 4

// --- Display ---
#define OLED_ADDR 0x3C
#define DISPLAY_WIDTH 128
#define DISPLAY_HEIGHT 64

// --- Timing ---
#define REQUEST_POOL_EXEC_INTERVAL_MS 250
#define FETCH_SPOTIFY_STATE_INTERVAL_MS 1000
#define REFRESH_TOKEN_INTERVAL_SECS 3600
#define DEBOUNCE_DELAY_MS 50

// --- Button Map ---
enum ButtonType {
  BTN_UP = 0,
  BTN_DOWN,
  BTN_LEFT,
  BTN_RIGHT,
  BTN_CONFIRM,
  BTN_COUNT // Helper to know array size
};

const int BTN_PINS[BTN_COUNT] = {
  10, // UP
  11, // DOWN
  13, // LEFT
  12, // RIGHT
  9   // CONFIRM
};