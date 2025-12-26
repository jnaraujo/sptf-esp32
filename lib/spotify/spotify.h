#pragma once

#include <Arduino.h>
#include <ArduinoJson.h>
#include <HTTPClient.h>
#include <base64.h>
#include <debug.h>

#include <expected>
#include <string>

struct PlaybackState {
	std::string title;
	std::string artist;
	std::string album;
	bool isPlaying;
	int volume_percent;
	int progress_ms;
	int duration_ms;
	unsigned long updated_at;

	PlaybackState() {
		title = "N/A";
		artist = "N/A";
		album = "N/A";
		isPlaying = false;
		volume_percent = 0;
		progress_ms = 0;
		duration_ms = 0;
		updated_at = 0;
	}
};

enum class SpotifyError { HttpError, ParseError };

class SpotifyClient {
public:
	SpotifyClient();
	~SpotifyClient();

	void refreshToken(const std::string& clientID, const std::string& clientSecret, const std::string& refreshToken);
	std::expected<PlaybackState, SpotifyError> fetchPlaybackState();
	void play();
	void pause();
	void next();
	void previous();
	void setVolume(int volume);

private:
	HTTPClient httpClient;

	std::string token;
	std::string SPOTIFY_API_BASE = "https://api.spotify.com";

	void prepareRequest(std::string endpoint);
};