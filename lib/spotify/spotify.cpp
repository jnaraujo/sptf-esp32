#include "spotify.h"

const String SPOTIFY_API = "https://api.spotify.com";

String Spotify::refreshToken(String clientID, String clientSecret, String refreshToken) {
  HTTPClient http;
  http.begin("https://accounts.spotify.com/api/token");

  http.addHeader("Authorization",  "Basic " + base64::encode(clientID+":"+clientSecret));
  http.addHeader("Content-Type", "application/x-www-form-urlencoded");

  String postData = "grant_type=refresh_token";
  postData += "&refresh_token=" + refreshToken;
  int httpCode = http.POST(postData);
  
  String newToken = "err";
  if (httpCode == 200) {
    String payload = http.getString();
    JsonDocument doc;
    DeserializationError error = deserializeJson(doc, payload);

    if (error) {
      Serial.print("Failed to parse JSON: ");
      Serial.println(error.c_str());
    } else {
      const char* access_token = doc["access_token"];
      if (access_token) {
        newToken = access_token;
      }
    }
  } else {
    Serial.printf("Request failed: %d\n", httpCode);
    String payload = http.getString();
    Serial.println("Response:");
    Serial.println(payload);
  }

  http.end();
  return newToken;
}

PlaybackState Spotify::fetchPlaybackState(String accessToken) {
  HTTPClient http;
  http.begin(SPOTIFY_API + "/v1/me/player?market=BR");
  http.addHeader("Authorization",  "Bearer " + accessToken);
  int status = http.GET();
  String payload = http.getString();
  http.end();

  PlaybackState ps;
  
  if(status != 200) {
    Serial.printf("HTTP Error: %d\n", status);
    Serial.println(payload);

    ps.title = "err";
    return ps;
  }

  JsonDocument doc;
  DeserializationError error = deserializeJson(doc, payload);
  if(error) {
    Serial.print("Erro ao parsear JSON: ");
    Serial.println(error.c_str());

    ps.title = "err";
    return ps;
  }

  ps.title = doc["item"]["name"].as<String>();
  ps.artist = doc["item"]["artists"][0]["name"].as<String>();
  ps.isPlaying = doc["is_playing"].as<bool>();
  ps.volume_percent = doc["device"]["volume_percent"].as<int>();
  return ps;
}

void Spotify::play(String accessToken) {
  HTTPClient http;
  http.begin(SPOTIFY_API + "/v1/me/player/play");
  http.addHeader("Authorization",  "Bearer " + accessToken);
  http.addHeader("Content-Length", "0");
  int status = http.PUT("");
  http.end();
  
  if(status != 200) {
    Serial.printf("HTTP Error: %d\n", status);
  }
}

void Spotify::pause(String accessToken) {
  HTTPClient http;
  http.begin(SPOTIFY_API + "/v1/me/player/pause");
  http.addHeader("Authorization",  "Bearer " + accessToken);
  http.addHeader("Content-Length", "0");
  int status = http.PUT("");
  http.end();
  
  if(status != 200) {
    Serial.printf("HTTP Error: %d\n", status);
  }
}

void Spotify::next(String accessToken) {
  HTTPClient http;
  http.begin(SPOTIFY_API + "/v1/me/player/next");
  http.addHeader("Authorization",  "Bearer " + accessToken);
  http.addHeader("Content-Length", "0");
  int status = http.POST("");
  http.end();
  
  if(status != 200) {
    Serial.printf("HTTP Error: %d\n", status);
  }
}

void Spotify::previous(String accessToken) {
  HTTPClient http;
  http.begin(SPOTIFY_API + "/v1/me/player/previous");
  http.addHeader("Authorization",  "Bearer " + accessToken);
  http.addHeader("Content-Length", "0");
  int status = http.POST("");
  http.end();
  
  if(status != 200) {
    Serial.printf("HTTP Error: %d\n", status);
  }
}

void Spotify::setVolume(String accessToken, int volume) {
  volume = max(0, min(volume, 100));

  HTTPClient http;
  http.begin(SPOTIFY_API + "/v1/me/player/volume?volume_percent=" + volume);
  http.addHeader("Authorization",  "Bearer " + accessToken);
  http.addHeader("Content-Length", "0");
  int status = http.PUT("");
  http.end();
  
  if(status != 204) {
    Serial.printf("HTTP Error: %d\n", status);
  }
}