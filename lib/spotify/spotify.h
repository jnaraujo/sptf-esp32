#pragma once

#include <HTTPClient.h>
#include <Arduino.h>
#include <ArduinoJson.h>
#include <base64.h>
#include <debug.h>

struct PlaybackState {
  String title;
  String artist;
  String album;
  bool isPlaying;
  int volume_percent;

   PlaybackState() {
    title = "N/A";
    artist = "N/A";
    album = "N/A";
    isPlaying = false;
    volume_percent = 0;
  }
};

class SpotifyClient {
  public:
    SpotifyClient();
    ~SpotifyClient();

    void refreshToken(const String& clientID,
                      const String& clientSecret,
                      const String& refreshToken);
    PlaybackState fetchPlaybackState();
    void play();
    void pause();
    void next();
    void previous();
    void setVolume(int volume);

  private:
    SemaphoreHandle_t mutex;
    HTTPClient httpClient;

    String token;
};