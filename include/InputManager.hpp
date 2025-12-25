#pragma once

#include <Arduino.h>

#include <array>

#include "Config.hpp"

struct ButtonState {
	int pin;
	int state;
	int lastState;
	uint32_t lastDebounceTime;
};

class InputManager {
private:
	std::array<ButtonState, Config::BTN_COUNT> buttons;

public:
	void begin();
	bool wasPressed(Config::ButtonType btnIndex);
};