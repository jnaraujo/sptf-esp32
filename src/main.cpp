#include <Arduino.h>
#include <array>
#include <WiFi.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <U8g2_for_Adafruit_GFX.h>
#include "secrets.h"
#include "spotify.h"
#include <Ticker.h>
#include <mutex>

#define delayFetchSpotifyState 1000
#define delayRefreshToken 3600
#define OLED_ADDR 0x3C
#define SCL 5
#define SDA 4
#define DISPLAY_WIDTH 128
#define DISPLAY_HEIGHT 64

const int BTN_PINS[5] = {
  10, // UP
  11, // DOWN
  13, // LEFT
  12, // RIGHT
  9 // CONFIRM
};

enum BTN_STATES {
  UP,
  DOWN,
  LEFT,
  RIGHT,
  CONFIRM
};

struct Button {
  int pin;
  int state;
  int lastState;
  uint32_t lastDebounceTime;
};

String wordWrap(String s, int limit);
String formatString(const String& s, int numLines, int maxCharPerLine);
int checkButton(int index, uint32_t currentMillis);
void fetchSpotifyState();
void refreshToken();
void backgroundTask(void *pvParameters);
String wifiStatusToString(wl_status_t status);

Adafruit_SSD1306 display(DISPLAY_WIDTH, DISPLAY_HEIGHT, &Wire, -1);
U8G2_FOR_ADAFRUIT_GFX u8g2_for_adafruit_gfx;

Ticker refreshTokenTicker;

std::mutex spotifyStateMutex;
PlaybackState spotifyState;
String spotifyToken = "";

uint32_t debounceDelay = 50;
std::array<Button, 5> buttons;

void setup() {
  Serial.begin(115200);

  Wire.begin(SDA, SCL);

  display.begin(SSD1306_SWITCHCAPVCC, OLED_ADDR);
  display.setTextWrap(false);
  display.setTextColor(SSD1306_WHITE);
  display.clearDisplay();

  u8g2_for_adafruit_gfx.begin(display);
  u8g2_for_adafruit_gfx.setFontMode(1);
  u8g2_for_adafruit_gfx.setFontDirection(0);
  u8g2_for_adafruit_gfx.setForegroundColor(SSD1306_WHITE);

  display.display();

  for (int i = 0; i < 5; i++){
    pinMode(BTN_PINS[i], INPUT_PULLUP);
    buttons[i] = {BTN_PINS[i], HIGH, HIGH, 0};
  }

  Serial.println("Connecting to Wi-Fi");
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  while (WiFi.status() != WL_CONNECTED) {
    Serial.printf("Status: %s\n", wifiStatusToString(WiFi.status()));
    delay(500);
  }
  Serial.println("Connected.");
  Serial.printf("IP Addr: %s\n", WiFi.localIP().toString().c_str());

  xTaskCreate(
    backgroundTask,
    "backgroundTask",
    10000,
    NULL,
    1,
    NULL
  );

  refreshToken();
  refreshTokenTicker.attach(delayRefreshToken, refreshToken);
}

void loop() {
  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("Reconnecting to WiFi...");
    WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
    delay(1000);
    return;
  }

  uint32_t currentMillis = millis();

  spotifyStateMutex.lock();
  int currentVolume = spotifyState.volume_percent;
  String title = spotifyState.title;
  String artist = spotifyState.artist;
  bool isPlaying = spotifyState.isPlaying;
  spotifyStateMutex.unlock();

  for (int i = 0; i < 5; i++){
    int reading = checkButton(i, currentMillis);

    if (reading) {
      Serial.printf("BTN ID: %d\n", i);
      switch (i) {
      case BTN_STATES::RIGHT:
        Spotify::next(spotifyToken);
        break;
      case BTN_STATES::LEFT:
        Spotify::previous(spotifyToken);
        break;
      case BTN_STATES::CONFIRM:
        if(isPlaying) {
          Spotify::pause(spotifyToken);
        } else {
          Spotify::play(spotifyToken);
        }
        break;
      case BTN_STATES::UP:
        Spotify::setVolume(spotifyToken, currentVolume + 10);
        break;
      case BTN_STATES::DOWN:
        Spotify::setVolume(spotifyToken, currentVolume - 10);
        break;
      default:
        break;
      }
    }
  }

  display.clearDisplay();

  u8g2_for_adafruit_gfx.setFont(u8g2_font_profont12_tf);
  u8g2_for_adafruit_gfx.setCursor(0, 10);
  u8g2_for_adafruit_gfx.print(artist);

  u8g2_for_adafruit_gfx.setFont(u8g2_font_profont17_tf);
  u8g2_for_adafruit_gfx.setCursor(0, 29);

  u8g2_for_adafruit_gfx.print(wordWrap(formatString(title, 2, 14), 14));

  u8g2_for_adafruit_gfx.setFont(u8g2_font_profont10_tf);
  u8g2_for_adafruit_gfx.setCursor(0, 64);
  u8g2_for_adafruit_gfx.print(isPlaying ? "Playing" : "Paused");

  display.display();
}

void refreshToken() {
  Serial.println("Refreshing access token");
  spotifyToken = Spotify::refreshToken(SPTF_CLIENT_ID, SPTF_CLIENT_SECRET, SPTF_REFRESH_TOKEN);
  if(spotifyToken == "err") {
    Serial.println("Err refreshToken");
  }
}

void fetchSpotifyState() {
  if(spotifyToken != ""){
    Serial.println("Fetching spotify state");
    PlaybackState newSpotifyState = Spotify::fetchPlaybackState(spotifyToken);
    if(newSpotifyState.title == "err") {
      Serial.println("Err fetchPlaybackState");
      return;
    }

    spotifyStateMutex.lock();
    spotifyState = newSpotifyState;
    spotifyStateMutex.unlock();
  }
}

void backgroundTask(void *pvParameters) {
  while (true) {
    fetchSpotifyState();
    delay(delayFetchSpotifyState);
  }
}

int checkButton(int index, uint32_t currentMillis) {
  Button *btn = &buttons.at(index);
  int reading = digitalRead(btn->pin);

  if (reading != btn->lastState) {
    btn->lastDebounceTime = currentMillis;
  }

  if ((currentMillis - btn->lastDebounceTime) > debounceDelay) {
    if (reading == LOW && btn->state == HIGH) {
      btn->state = LOW;
      return 1;  // Button was pressed
    } else if (reading == HIGH && btn->state == LOW) {
      btn->state = HIGH;
    }
  }

  btn->lastState = reading;
  return 0;  // No button press detected
}

String formatString(const String& s, int numLines, int maxCharPerLine) {
  const int totalMaxChars = maxCharPerLine * numLines;
  const String ellipsis = "...";

  if(s.length() <= totalMaxChars) {
    return s;
  }

  return s.substring(0, totalMaxChars - ellipsis.length()) + ellipsis;
}

String wordWrap(String s, int limit) {
  int space = 0;
  int i = 0;
  int line = 0;
  while (i < s.length()) {
    if (s.substring(i, i + 1) == " ") {
      space = i;
    }
    if (line > limit - 1) {
      s = s.substring(0, space) + "~" + s.substring(space + 1);
      line = 0;
    }
    i++; line++;
  }
  s.replace("~", "\n");
  return s;
}

String wifiStatusToString(wl_status_t status) {
  switch (status) {
    case WL_NO_SHIELD:
      return "No WiFi shield is present";
    case WL_IDLE_STATUS:
      return "Idle Status";
    case WL_NO_SSID_AVAIL:
      return "No SSID available";
    case WL_SCAN_COMPLETED:
      return "Scan Completed";
    case WL_CONNECTED:
      return "Connected";
    case WL_CONNECT_FAILED:
      return "Connection Failed";
    case WL_CONNECTION_LOST:
      return "Connection Lost";
    case WL_DISCONNECTED:
      return "Disconnected";
    default:
      return "Unknown Status";
  }
}