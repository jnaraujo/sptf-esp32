#pragma once

#include <Arduino.h>
#include <ArduinoJson.h>
#include <HTTPClient.h>
#include <base64.h>
#include <debug.h>

struct PlaybackState {
	String title;
	String artist;
	String album;
	bool isPlaying;
	int volume_percent;
	int progress_ms;
	int duration_ms;

	PlaybackState() {
		title = "N/A";
		artist = "N/A";
		album = "N/A";
		isPlaying = false;
		volume_percent = 0;
		progress_ms = 0;
		duration_ms = 0;
	}
};

class SpotifyClient {
public:
	SpotifyClient();
	~SpotifyClient();

	void refreshToken(const String& clientID, const String& clientSecret, const String& refreshToken);
	PlaybackState fetchPlaybackState();
	void play();
	void pause();
	void next();
	void previous();
	void setVolume(int volume);

private:
	HTTPClient httpClient;

	String token;
	String SPOTIFY_API_BASE = "https://api.spotify.com";

	void prepareRequest(String endpoint);
};