#include <Arduino.h>
#include <array>
#include <functional>
#include <WiFi.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <U8g2_for_Adafruit_GFX.h>
#include "secrets.h"
#include <spotify.h>
#include <Ticker.h>
#include <debug.h>

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
String centerString(const String& text, int totalWidth);
String wifiStatusToString(wl_status_t status);

Adafruit_SSD1306 display(DISPLAY_WIDTH, DISPLAY_HEIGHT, &Wire, -1);
U8G2_FOR_ADAFRUIT_GFX u8g2_for_adafruit_gfx;

Ticker refreshTokenTicker;

SemaphoreHandle_t spotifyStateMutex;
PlaybackState spotifyState;
SemaphoreHandle_t spotifyTokenMutex;
String spotifyToken = "";

String getSpotifyToken() {
  xSemaphoreTake(spotifyTokenMutex, portMAX_DELAY);
  auto tkn = spotifyToken;
  xSemaphoreGive(spotifyTokenMutex);
  return tkn;
}

uint32_t debounceDelay = 50;
std::array<Button, 5> buttons;
uint32_t lastBlink = millis();
bool shouldBlink = false;
bool shouldRefreshToken = true;

QueueHandle_t requestPool = xQueueCreate(10, sizeof(std::function<void()>));

void addRequestToPool(std::function<void()> fn) {
  xQueueSend(requestPool, &fn, portMAX_DELAY);
}

void setup() {
  Serial.begin(115200);

  spotifyStateMutex = xSemaphoreCreateMutex();
  spotifyTokenMutex = xSemaphoreCreateMutex();


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

  DEBUG_PRINTLN("Connecting to Wi-Fi");
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  while (WiFi.status() != WL_CONNECTED) {
    DEBUG_PRINTF("Status: %s\n", wifiStatusToString(WiFi.status()).c_str());
    delay(500);
  }
  DEBUG_PRINTLN("Connected.");
  DEBUG_PRINTF("IP Addr: %s\n", WiFi.localIP().toString().c_str());

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
    DEBUG_PRINTLN("Reconnecting to WiFi...");
    WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
    delay(1000);
    return;
  }

  if(shouldRefreshToken) {
    DEBUG_PRINTLN("Refreshing access token");
    String token = Spotify::refreshToken(SPTF_CLIENT_ID, SPTF_CLIENT_SECRET, SPTF_REFRESH_TOKEN);
    if(token == "err") {
      DEBUG_PRINTLN("Err refreshToken");
    } else {
      xSemaphoreTake(spotifyTokenMutex, portMAX_DELAY);
      spotifyToken = token;
      xSemaphoreGive(spotifyTokenMutex);
    }
    shouldRefreshToken = false;
  }

  uint32_t currentMillis = millis();

  xSemaphoreTake(spotifyStateMutex, portMAX_DELAY);
  int currentVolume = spotifyState.volume_percent;
  String title = spotifyState.title;
  String artist = spotifyState.artist;
  String album = spotifyState.album;
  bool isPlaying = spotifyState.isPlaying;
  xSemaphoreGive(spotifyStateMutex);

  for (int i = 0; i < 5; i++){
    int reading = checkButton(i, currentMillis);

    if (reading) {
      DEBUG_PRINTF("BTN ID: %d\n", i);
      switch (i) {
      case BTN_STATES::RIGHT:
        addRequestToPool([=]() {
          Spotify::next(getSpotifyToken());
        });
        break;
      case BTN_STATES::LEFT:
        addRequestToPool([=]() {
          Spotify::previous(getSpotifyToken());
        });
        break;
      case BTN_STATES::CONFIRM:
        addRequestToPool([=]() {
          if(isPlaying) {
            Spotify::pause(getSpotifyToken());
          } else {
            Spotify::play(getSpotifyToken());
          }
        });
        break;
      case BTN_STATES::UP:
        xSemaphoreTake(spotifyStateMutex, portMAX_DELAY);
        spotifyState.volume_percent = currentVolume + 10;
        xSemaphoreGive(spotifyStateMutex);
        addRequestToPool([=]() {
          Spotify::setVolume(getSpotifyToken(), currentVolume + 10);
        });
        break;
      case BTN_STATES::DOWN:
        xSemaphoreTake(spotifyStateMutex, portMAX_DELAY);
        spotifyState.volume_percent = currentVolume - 10;
        xSemaphoreGive(spotifyStateMutex);
        addRequestToPool([=]() {
          Spotify::setVolume(getSpotifyToken(), currentVolume - 10);
        });
        break;
      default:
        break;
      }
    }
  }

  display.clearDisplay();

  u8g2_for_adafruit_gfx.setFont(u8g2_font_profont10_tf);
  u8g2_for_adafruit_gfx.setCursor(0, 10);

  String line = formatString(artist, 1, 14) + " - " + album;
  u8g2_for_adafruit_gfx.print(
    formatString(
      line,
    1, 24)
  );

  u8g2_for_adafruit_gfx.setFont(u8g2_font_profont17_tf);
  u8g2_for_adafruit_gfx.setCursor(0, 29);

  u8g2_for_adafruit_gfx.print(wordWrap(formatString(title, 2, 14), 14));

  if (currentMillis - lastBlink > 500) {
    shouldBlink = !shouldBlink;
    lastBlink = currentMillis;
  }

  String playingTxt = "(^-^)/";
  String pausedTxt = "(-.-) zZ";
  String pad = "=-=-=-=";
  if(shouldBlink) {
    pad = "-=-=-=-";
    if(isPlaying) {
      playingTxt = "(^O^)_";
    } else {
      pausedTxt = "(-.-) Zz";
    }
  }

  u8g2_for_adafruit_gfx.setFont(u8g2_font_profont10_tf);
  u8g2_for_adafruit_gfx.setCursor(0, 60);
  u8g2_for_adafruit_gfx.printf("%s %s %s", pad.c_str(), centerString(isPlaying ? playingTxt : pausedTxt, 10).c_str(), pad.c_str());


  display.display();
}

void refreshToken() {
  shouldRefreshToken = true;
}

void fetchSpotifyState() {
  if(getSpotifyToken() != ""){
    DEBUG_PRINTLN("Fetching spotify state");
    PlaybackState newSpotifyState = Spotify::fetchPlaybackState(getSpotifyToken());
    if(newSpotifyState.title == "err") {
      DEBUG_PRINTLN("Err fetchPlaybackState");
      return;
    }

    xSemaphoreTake(spotifyStateMutex, portMAX_DELAY);
    spotifyState = newSpotifyState;
    xSemaphoreGive(spotifyStateMutex);
  }
}

void backgroundTask(void *pvParameters) {
  while (true) {
    uint32_t start = millis();

    UBaseType_t poolSize = uxQueueMessagesWaiting(requestPool);
    UBaseType_t toProcess = std::min<UBaseType_t>(poolSize, 3);

    std::function<void()> fn;
    for (UBaseType_t i = 0; i < poolSize; ++i) {
      if (xQueueReceive(requestPool, &fn, 0) == pdTRUE) {
        fn();
      }
    }
    uint32_t end = millis();

    uint32_t elapsed = end - start;
    uint32_t wait = (delayFetchSpotifyState > elapsed) ? (delayFetchSpotifyState - elapsed) : 0;
    DEBUG_PRINTF(
      "Pool Size: %u; Pool Exec: %lu ms; Wait: %lu ms\n",
      poolSize, elapsed, wait
    );

    fetchSpotifyState();
    delay(wait);
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

String centerString(const String& text, int totalWidth) {
  int textLen = text.length();
  if (textLen >= totalWidth) {
    return text;
  }
  int totalPadding = totalWidth - textLen;
  int leftPadding = totalPadding / 2;
  int rightPadding = totalPadding - leftPadding;
  String paddedString = "";
  for (int i = 0; i < leftPadding; i++) {
    paddedString += " ";
  }
  paddedString += text;
  for (int i = 0; i < rightPadding; i++) {
    paddedString += " ";
  }
  return paddedString;
}

String formatString(const String& s, int numLines, int maxCharPerLine) {
  const int totalMaxChars = maxCharPerLine * numLines;
  const String ellipsis = "...";

  if(s.length() <= totalMaxChars) {
    return s;
  }

  String trimmed = s.substring(0, totalMaxChars - ellipsis.length());
  if (trimmed.endsWith(" ")) {
    trimmed.remove(trimmed.length() - 1);
  }
  return trimmed + ellipsis;
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