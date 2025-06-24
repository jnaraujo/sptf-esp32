#pragma once

#include <HTTPClient.h>
#include <Arduino.h>
#include <ArduinoJson.h>
#include <base64.h>
#include "debug.h"

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

namespace Spotify {
  PlaybackState fetchPlaybackState(String accessToken);
  String refreshToken(String clientID, String clientSecret, String refreshToken);

  void play(String accessToken);
  void pause(String accessToken);
  void next(String accessToken);
  void previous(String accessToken);
  void setVolume(String accessToken, int volume);
}