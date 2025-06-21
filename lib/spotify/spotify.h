#pragma once

#include <HTTPClient.h>
#include <Arduino.h>
#include <ArduinoJson.h>
#include <base64.h>

struct PlaybackState {
  String title;
  String artist;
  bool isPlaying;
  int volume_percent;

   PlaybackState() {
    title = "Hello!";
    artist = ":p";
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