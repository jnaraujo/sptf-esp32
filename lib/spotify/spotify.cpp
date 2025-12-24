#include "spotify.h"

SpotifyClient::SpotifyClient() {}

SpotifyClient::~SpotifyClient() {}

void SpotifyClient::refreshToken(const String& clientID, const String& clientSecret, const String& refreshToken) {
    HTTPClient http;

    http.begin("https://accounts.spotify.com/api/token");

    http.addHeader("Authorization", "Basic " + base64::encode(clientID + ":" + clientSecret));
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

    JsonDocument doc(2000);
    DeserializationError error = deserializeJson(doc, payload);

    if (error) {
        DEBUG_PRINTF("Failed to parse JSON: %s\n", error.c_str());
        return;
    }

    this->token = doc["access_token"].as<String>();
}

PlaybackState SpotifyClient::fetchPlaybackState() {
    this->prepareRequest("/v1/me/player?market=BR");

    int status = this->httpClient.GET();

    PlaybackState ps;

    if (status == 204) {
        this->httpClient.end();

        ps.title = "Not Playing";
        ps.artist = "(o_O)";
        return ps;
    }

    String payload = this->httpClient.getString();
    this->httpClient.end();

    if (status != 200) {
        DEBUG_PRINTF("HTTP Error: %d\n", status);
        DEBUG_PRINTLN(payload);
        return ps;
    }

    JsonDocument doc;
    DeserializationError error = deserializeJson(doc, payload);
    if (error) {
        DEBUG_PRINT("Erro ao parsear JSON: ");
        DEBUG_PRINTLN(error.c_str());
        return ps;
    }

    ps.title = doc["item"]["name"].as<String>();
    ps.artist = doc["item"]["artists"][0]["name"].as<String>();
    ps.album = doc["item"]["album"]["name"].as<String>();
    ps.isPlaying = doc["is_playing"].as<bool>();
    ps.volume_percent = doc["device"]["volume_percent"].as<int>();
    ps.progress_ms = doc["progress_ms"].as<int>();
    ps.duration_ms = doc["item"]["duration_ms"].as<int>();
    return ps;
}

void SpotifyClient::play() {
    this->prepareRequest("/v1/me/player/play");

    this->httpClient.addHeader("Content-Length", "0");

    int status = this->httpClient.PUT("");
    this->httpClient.end();

    if (status != 200) {
        DEBUG_PRINTF("HTTP Error: %d\n", status);
    }
}

void SpotifyClient::pause() {
    this->prepareRequest("/v1/me/player/pause");

    this->httpClient.addHeader("Content-Length", "0");

    int status = this->httpClient.PUT("");
    this->httpClient.end();

    if (status != 200) {
        DEBUG_PRINTF("HTTP Error: %d\n", status);
    }
}

void SpotifyClient::next() {
    this->prepareRequest("/v1/me/player/next");

    this->httpClient.addHeader("Content-Length", "0");

    int status = this->httpClient.POST("");
    this->httpClient.end();

    if (status != 200) {
        DEBUG_PRINTF("HTTP Error: %d\n", status);
    }
}

void SpotifyClient::previous() {
    this->prepareRequest("/v1/me/player/previous");

    this->httpClient.addHeader("Content-Length", "0");

    int status = this->httpClient.POST("");
    this->httpClient.end();

    if (status != 200) {
        DEBUG_PRINTF("HTTP Error: %d\n", status);
    }
}

void SpotifyClient::setVolume(int volume) {
    volume = max(0, min(volume, 100));

    this->prepareRequest("/v1/me/player/volume?volume_percent=" + String(volume));

    this->httpClient.addHeader("Content-Length", "0");

    int status = this->httpClient.PUT("");
    this->httpClient.end();

    if (status != 204) {
        DEBUG_PRINTF("HTTP Error: %d\n", status);
    }
}

void SpotifyClient::prepareRequest(String endpoint) {
    this->httpClient.begin(SPOTIFY_API_BASE + endpoint);

    this->httpClient.addHeader("Authorization", "Bearer " + this->token);
    this->httpClient.setReuse(true);
    this->httpClient.setTimeout(2000);
}