#include "InputManager.hpp"

void InputManager::begin() {
    for (int i = 0; i < BTN_COUNT; i++) {
        pinMode(BTN_PINS[i], INPUT_PULLUP);
        buttons[i] = {BTN_PINS[i], HIGH, HIGH, 0};
    }
}

bool InputManager::wasPressed(ButtonType index) {
    ButtonState &btn = buttons.at(index);
    int reading = digitalRead(btn.pin);
    bool pressed = false;
    uint32_t currentMillis = millis();

    if (reading != btn.lastState) {
        btn.lastDebounceTime = currentMillis;
    }

    if ((currentMillis - btn.lastDebounceTime) > DEBOUNCE_DELAY_MS) {
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