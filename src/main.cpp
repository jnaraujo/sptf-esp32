#include <Arduino.h>
#include <WiFi.h>
#include <Wire.h>
#include "Config.hpp"
#include "secrets.h"
#include "spotify.h"
#include "debug.h"
#include "NetworkUtils.hpp"
#include "InputManager.hpp"
#include "DisplayManager.hpp"

// Type definitions
enum class SpotifyCmd {
    NONE,
    NEXT,
    PREVIOUS,
    PLAY,
    PAUSE,
    SET_VOLUME
};

struct SpotifyRequest {
    SpotifyCmd cmd;
    int value;
};

// --- Global Objects ---
SpotifyClient spotifyClient;
InputManager inputManager;
DisplayManager displayManager;

// --- Concurrency ---
SemaphoreHandle_t spotifyStateMutex;
PlaybackState spotifyState;
QueueHandle_t requestPool = xQueueCreate(6, sizeof(SpotifyRequest));

// --- Timers ---
uint32_t lastRequestPoolExecMillis = 0;
uint32_t lastFetchSpotifyStateMillis = 0;
uint32_t lastRefreshTokenMillis = 0;

// --- Prototypes ---
void backgroundTask(void *pvParameters);
void handleButtonPress(ButtonType btn, const PlaybackState& currentState);
void fetchSpotifyState();


void addRequestToPool(SpotifyCmd cmd, int value = 0) {
    SpotifyRequest req;
    req.cmd = cmd;
    req.value = value;
    xQueueSend(requestPool, &req, portMAX_DELAY);
}

void setup() {
  Serial.begin(115200);
  Wire.begin(OLED_SDA, OLED_SCL);

  // Init Modules
  inputManager.begin();
  displayManager.begin();
  spotifyStateMutex = xSemaphoreCreateMutex();

  DEBUG_PRINTLN("Connecting to Wi-Fi");
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
  }
  DEBUG_PRINTLN("Connected.");

  spotifyClient.refreshToken(SPTF_CLIENT_ID, SPTF_CLIENT_SECRET, SPTF_REFRESH_TOKEN);

  xTaskCreate(backgroundTask, "bgTask", 10000, NULL, 1, NULL);
}

void loop() {
  if (WiFi.status() != WL_CONNECTED) return;

  PlaybackState currentState;
  if (xSemaphoreTake(spotifyStateMutex, portMAX_DELAY)) {
      currentState = spotifyState;
      xSemaphoreGive(spotifyStateMutex);
  }

  for (int i = 0; i < BTN_COUNT; i++) {
    ButtonType btn = static_cast<ButtonType>(i);
    if (inputManager.wasPressed(btn)) {
        handleButtonPress(btn, currentState);
    }
  }

  displayManager.render(currentState);
}

void handleButtonPress(ButtonType btn, const PlaybackState& currentState) {
    DEBUG_PRINTF("BTN ID: %d\n", btn);

    switch (btn) {
        case BTN_RIGHT:   addRequestToPool(SpotifyCmd::NEXT); break;
        case BTN_LEFT:    addRequestToPool(SpotifyCmd::PREVIOUS); break;
        case BTN_CONFIRM: addRequestToPool(currentState.isPlaying ? SpotifyCmd::PAUSE : SpotifyCmd::PLAY); break;
        case BTN_UP:      addRequestToPool(SpotifyCmd::SET_VOLUME, currentState.volume_percent + 10); break;
        case BTN_DOWN:    addRequestToPool(SpotifyCmd::SET_VOLUME, currentState.volume_percent - 10); break;
        default: break;
    }
}


void backgroundTask(void *pvParameters) {
  while (true) {
    long timeUntilFetch = (lastFetchSpotifyStateMillis + FETCH_SPOTIFY_STATE_INTERVAL_MS) - millis();
    long timeUntilToken = (lastRefreshTokenMillis + REFRESH_TOKEN_INTERVAL_SECS * 1000) - millis();

    if (timeUntilFetch < 0) timeUntilFetch = 0;
    if (timeUntilToken < 0) timeUntilToken = 0;

    uint32_t timeToWaitMs = std::min(timeUntilFetch, timeUntilToken);

    SpotifyRequest req;
    if (xQueueReceive(requestPool, &req, pdMS_TO_TICKS(timeToWaitMs)) == pdTRUE) {
      DEBUG_PRINTLN("Processing request from pool");
      uint32_t tStart = millis();
      switch (req.cmd) {
        case SpotifyCmd::NEXT:       spotifyClient.next(); break;
        case SpotifyCmd::PREVIOUS:   spotifyClient.previous(); break;
        case SpotifyCmd::PLAY:       spotifyClient.play(); break;
        case SpotifyCmd::PAUSE:      spotifyClient.pause(); break;
        case SpotifyCmd::SET_VOLUME: spotifyClient.setVolume(req.value); break;
        default: break;
      }
      DEBUG_PRINTF("Request processed in %lu ms\n", millis() - tStart);
    }

    if((millis() - lastFetchSpotifyStateMillis) > FETCH_SPOTIFY_STATE_INTERVAL_MS) {
      uint32_t start = millis();
      fetchSpotifyState();

      DEBUG_PRINTF(
        "Fetch Spotify State: %lu ms\n",
        millis() - start
      );

      lastFetchSpotifyStateMillis = millis();
    }

    if((millis() - lastRefreshTokenMillis) > REFRESH_TOKEN_INTERVAL_SECS * 1000) {
      DEBUG_PRINTLN("Refreshing access token");
      spotifyClient.refreshToken(SPTF_CLIENT_ID, SPTF_CLIENT_SECRET, SPTF_REFRESH_TOKEN);
      lastRefreshTokenMillis = millis();
    }
  }
}

void fetchSpotifyState() {
  DEBUG_PRINTLN("Fetching spotify state");
  PlaybackState newSpotifyState = spotifyClient.fetchPlaybackState();
  if(newSpotifyState.title == "err") {
    DEBUG_PRINTLN("Err fetchPlaybackState");
    return;
  }

  xSemaphoreTake(spotifyStateMutex, portMAX_DELAY);
  spotifyState = newSpotifyState;
  xSemaphoreGive(spotifyStateMutex);
}