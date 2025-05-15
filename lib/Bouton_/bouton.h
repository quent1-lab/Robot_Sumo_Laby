#pragma once
#include <Arduino.h>

class Bouton {
public:
    enum EtatBouton {
        Idle = 0,
        Click,
        Press,
        Hold,
        DoubleClick
    };

    Bouton();

    void begin(int pin, bool activeLow = true, int clickDelay = 150, int pressDelay = 500, int debounceDelay = 30, int doubleClickDelay = 400);
    void update();

    bool wasClicked();
    bool wasDoubleClicked();
    bool wasPressed();
    bool isHeld();
    EtatBouton getState();

private:
    int pin;
    bool activeLow;
    int state = Idle;
    int lastStableState = HIGH;

    unsigned long lastChangeTime = 0;
    unsigned long lastPressTime = 0;
    unsigned long lastReleaseTime = 0;
    unsigned long lastClickTime = 0;

    int delayClick;
    int delayPress;
    int delayDebounce;
    int delayDoubleClick;

    bool waitingSecondClick = false;
    bool held = false;

    int readRaw();
    bool debouncedRead();
};
