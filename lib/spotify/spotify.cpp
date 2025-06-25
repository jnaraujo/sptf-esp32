#include "spotify.h"

const String SPOTIFY_API_BASE = "https://api.spotify.com";

void setupHTTPClient(HTTPClient& http, String endpoint, const String& accessToken) {
  http.begin(SPOTIFY_API_BASE + endpoint);
  http.addHeader("Authorization", "Bearer " + accessToken);
  http.setTimeout(1000);
}

SpotifyClient::SpotifyClient() {
  this->mutex = xSemaphoreCreateMutex();
  this->httpClient.setReuse(true);
}

SpotifyClient::~SpotifyClient() {
  vSemaphoreDelete(this->mutex);
}

void SpotifyClient::refreshToken(const String& clientID, const String& clientSecret, const String& refreshToken) {
  HTTPClient http;
  http.begin("https://accounts.spotify.com/api/token");

  http.addHeader("Authorization",  "Basic " + base64::encode(clientID+":"+clientSecret));
  http.addHeader("Content-Type", "application/x-www-form-urlencoded");

  String postData = "grant_type=refresh_token";
  postData += "&refresh_token=" + refreshToken;
  int httpCode = http.POST(postData);
  String payload = http.getString();
  http.end();

  if (httpCode != 200) {
    DEBUG_PRINTF("Request failed: %d\n", httpCode);
    return;
  }

  JsonDocument doc;
  DeserializationError error = deserializeJson(doc, payload);

  if (error) {
    DEBUG_PRINTF("Failed to parse JSON: %s\n", error.c_str());
    return;
  }

  xSemaphoreTake(this->mutex, portMAX_DELAY);
  this->token = doc["access_token"].as<String>();
  xSemaphoreGive(this->mutex);
}

PlaybackState SpotifyClient::fetchPlaybackState() {
  xSemaphoreTake(this->mutex, portMAX_DELAY);
  setupHTTPClient(this->httpClient, "/v1/me/player?market=BR", this->token);
  int status = this->httpClient.GET();
  
  PlaybackState ps;
  
  if(status == 204) {
    this->httpClient.end();
    xSemaphoreGive(this->mutex);

    ps.title = "Not Playing";
    ps.artist = "(o_O)";
    return ps;
  }

  String payload = this->httpClient.getString();
  this->httpClient.end();
  xSemaphoreGive(this->mutex);

  if(status != 200) {
    DEBUG_PRINTF("HTTP Error: %d\n", status);
    DEBUG_PRINTLN(payload);
    return ps;
  }

  JsonDocument doc;
  DeserializationError error = deserializeJson(doc, payload);
  if(error) {
    DEBUG_PRINT("Erro ao parsear JSON: ");
    DEBUG_PRINTLN(error.c_str());
    return ps;
  }

  ps.title = doc["item"]["name"].as<String>();
  ps.artist = doc["item"]["artists"][0]["name"].as<String>();
  ps.album = doc["item"]["album"]["name"].as<String>();
  ps.isPlaying = doc["is_playing"].as<bool>();
  ps.volume_percent = doc["device"]["volume_percent"].as<int>();
  return ps;
}

void SpotifyClient::play() {
  xSemaphoreTake(this->mutex, portMAX_DELAY);
  setupHTTPClient(this->httpClient, "/v1/me/player/play", this->token);
  this->httpClient.addHeader("Content-Length", "0");
  
  int status = this->httpClient.PUT("");
  this->httpClient.end();
  xSemaphoreGive(this->mutex);
  
  if(status != 200) {
    DEBUG_PRINTF("HTTP Error: %d\n", status);
  }
}

void SpotifyClient::pause() {
  xSemaphoreTake(this->mutex, portMAX_DELAY);
  setupHTTPClient(this->httpClient, "/v1/me/player/pause", this->token);
  this->httpClient.addHeader("Content-Length", "0");
  
  int status = this->httpClient.PUT("");
  this->httpClient.end();
  xSemaphoreGive(this->mutex);
  
  if(status != 200) {
    DEBUG_PRINTF("HTTP Error: %d\n", status);
  }
}

void SpotifyClient::next() {
  xSemaphoreTake(this->mutex, portMAX_DELAY);
  setupHTTPClient(this->httpClient, "/v1/me/player/next", this->token);
  this->httpClient.addHeader("Content-Length", "0");
  
  int status = this->httpClient.POST("");
  this->httpClient.end();
  xSemaphoreGive(this->mutex);
  
  if(status != 200) {
    DEBUG_PRINTF("HTTP Error: %d\n", status);
  }
}

void SpotifyClient::previous() {
  xSemaphoreTake(this->mutex, portMAX_DELAY);
  setupHTTPClient(this->httpClient, "/v1/me/player/previous", this->token);
  this->httpClient.addHeader("Content-Length", "0");
  
  int status = this->httpClient.POST("");
  this->httpClient.end();
  xSemaphoreGive(this->mutex);
  
  if(status != 200) {
    DEBUG_PRINTF("HTTP Error: %d\n", status);
  }
}

void SpotifyClient::setVolume(int volume) {
  xSemaphoreTake(this->mutex, portMAX_DELAY);
  volume = max(0, min(volume, 100));
  
  setupHTTPClient(this->httpClient, "/v1/me/player/volume?volume_percent=" + String(volume), this->token);
  this->httpClient.addHeader("Content-Length", "0");
  
  int status = this->httpClient.PUT("");
  this->httpClient.end();
  xSemaphoreGive(this->mutex);
  
  if(status != 204) {
    DEBUG_PRINTF("HTTP Error: %d\n", status);
  }
}