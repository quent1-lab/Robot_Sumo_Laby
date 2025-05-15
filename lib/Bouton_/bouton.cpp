#include "bouton.h"

Bouton::Bouton() {}

void Bouton::begin(int pin, bool activeLow, int clickDelay, int pressDelay, int debounceDelay, int doubleClickDelay) {
    this->pin = pin;
    this->activeLow = activeLow;
    this->delayClick = clickDelay;
    this->delayPress = pressDelay;
    this->delayDebounce = debounceDelay;
    this->delayDoubleClick = doubleClickDelay;
    pinMode(pin, INPUT);
}

int Bouton::readRaw() {
    return digitalRead(pin);
}

bool Bouton::debouncedRead() {
    int val = readRaw();
    if (val != lastStableState) {
        if (millis() - lastChangeTime > delayDebounce) {
            lastStableState = val;
            lastChangeTime = millis();
            return true;
        }
    }
    return false;
}

void Bouton::update() {
    int reading = readRaw();
    //Serial.println(reading);
    bool pressed = (reading == (activeLow ? LOW : HIGH));

    if (debouncedRead()) {
        if (pressed) {
            lastPressTime = millis();
            held = false;
        } else {
            unsigned long now = millis();
            if (now - lastPressTime < delayPress) {
                if (waitingSecondClick && now - lastClickTime < delayDoubleClick) {
                    state = DoubleClick;
                    waitingSecondClick = false;
                } else {
                    waitingSecondClick = true;
                    lastClickTime = now;
                    state = Click;
                }
            } else {
                state = Press;
            }
        }
    }

    if (pressed && !held && millis() - lastPressTime >= delayPress) {
        held = true;
        state = Hold;
    }

    if (waitingSecondClick && millis() - lastClickTime > delayDoubleClick) {
        waitingSecondClick = false;
    }
}

bool Bouton::wasClicked() {
    if (state == Click) {
        state = Idle;
        return true;
    }
    return false;
}

bool Bouton::wasDoubleClicked() {
    if (state == DoubleClick) {
        state = Idle;
        return true;
    }
    return false;
}

bool Bouton::wasPressed() {
    if (state == Press) {
        state = Idle;
        return true;
    }
    return false;
}

bool Bouton::isHeld() {
    return state == Hold;
}

Bouton::EtatBouton Bouton::getState() {
    return static_cast<EtatBouton>(state);
}
