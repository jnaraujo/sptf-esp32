#include "spotify.h"

SpotifyClient::SpotifyClient() {}

SpotifyClient::~SpotifyClient() {}

void SpotifyClient::refreshToken(const std::string& clientID, const std::string& clientSecret,
								 const std::string& refreshToken) {
	HTTPClient http;

	http.begin("https://accounts.spotify.com/api/token");

	std::string authRaw = clientID + ":" + clientSecret;
	String encodedAuth = base64::encode(String(authRaw.c_str()));

	http.addHeader("Authorization", "Basic " + encodedAuth);
	http.addHeader("Content-Type", "application/x-www-form-urlencoded");

	std::string postData;
	postData.reserve(150);
	postData = "grant_type=refresh_token&refresh_token=";
	postData += refreshToken;

	int httpCode = http.POST((uint8_t*)postData.c_str(), postData.size());

	if (httpCode != 200) {
		http.end();
		DEBUG_PRINTF("Request failed: %d\n", httpCode);
		return;
	}

	JsonDocument doc;
	DeserializationError error = deserializeJson(doc, http.getStream());

	http.end();

	if (error) {
		DEBUG_PRINTF("Failed to parse JSON: %s\n", error.c_str());
		return;
	}

	this->token = doc["access_token"].as<std::string>();
}

std::expected<PlaybackState, SpotifyError> SpotifyClient::fetchPlaybackState() {
	this->prepareRequest("/v1/me/player?market=BR");

	int status = this->httpClient.GET();

	PlaybackState ps;

	if (status == 204) {
		this->httpClient.end();

		ps.title = "Not Playing";
		ps.artist = "(o_O)";
		return ps;
	}

	if (status != 200) {
		this->httpClient.end();
		DEBUG_PRINTF("HTTP Error: %d\n", status);
		return std::unexpected(SpotifyError::HttpError);
	}

	JsonDocument doc;
	auto error = deserializeJson(doc, this->httpClient.getStream());

	this->httpClient.end();

	if (error) {
		DEBUG_PRINT("Erro ao parsear JSON: ");
		DEBUG_PRINTLN(error.c_str());
		return std::unexpected(SpotifyError::ParseError);
	}

	ps.title = doc["item"]["name"].as<std::string>();
	ps.artist = doc["item"]["artists"][0]["name"].as<std::string>();
	ps.album = doc["item"]["album"]["name"].as<std::string>();
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

	this->prepareRequest("/v1/me/player/volume?volume_percent=" + std::to_string(volume));

	this->httpClient.addHeader("Content-Length", "0");

	int status = this->httpClient.PUT("");
	this->httpClient.end();

	if (status != 204) {
		DEBUG_PRINTF("HTTP Error: %d\n", status);
	}
}

void SpotifyClient::prepareRequest(std::string endpoint) {
	auto fullUrl = std::string(SPOTIFY_API_BASE) + endpoint;

	this->httpClient.begin(fullUrl.c_str());

	std::string authValue = "Bearer " + this->token;
	this->httpClient.addHeader("Authorization", authValue.c_str());
	this->httpClient.setReuse(true);
	this->httpClient.setTimeout(2000);
}