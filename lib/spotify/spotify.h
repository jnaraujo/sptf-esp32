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