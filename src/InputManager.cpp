#include "InputManager.hpp"

// NOLINTNEXTLINE(readability-convert-member-functions-to-static)
auto InputManager::begin() -> void {
	for (size_t i = 0; i < Config::BTN_COUNT; i++) {
		pinMode(Config::BTN_PINS[i], INPUT_PULLUP);
		this->buttons[i] = {Config::BTN_PINS[i], HIGH, HIGH, 0};
	}
}

auto InputManager::wasPressed(Config::ButtonType index) -> bool {
	ButtonState& btn = this->buttons.at(index);
	int reading = digitalRead(btn.pin);
	bool pressed = false;
	uint32_t currentMillis = millis();

	if (reading != btn.lastState) {
		btn.lastDebounceTime = currentMillis;
	}

	if ((currentMillis - btn.lastDebounceTime) > Config::DEBOUNCE_DELAY_MS) {
		if (reading == LOW && btn.state == HIGH) {
			btn.state = LOW;
			pressed = true;
		} else if (reading == HIGH && btn.state == LOW) {
			btn.state = HIGH;
		}
	}

	btn.lastState = reading;
	return pressed;
}