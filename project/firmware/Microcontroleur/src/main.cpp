#include <Arduino.h>
#include "config.h"

constexpr int PRESSED_LEVEL = LOW;           // niveau lu quand on appuie (étape 1)
constexpr unsigned long DEBOUNCE_MS = 30;
constexpr int JOYSTICK_DEADZONE = 300;
int joystickCenterX = 2048;

struct Button {
    int pin;
    int stableLevel;              // dernier niveau confirmé
    int lastRead;                 // dernière lecture brute
    unsigned long lastChangeMs;   // date du dernier mouvement du signal
};

Button button = {PIN_BUTTON, !PRESSED_LEVEL, !PRESSED_LEVEL, 0};

// Renvoie true une seule fois par appui, au moment où il est confirmé
bool justPressed(Button& b) {
    int level = digitalRead(b.pin);
    if (level != b.lastRead) {         // le signal bouge : rebond ou vrai changement
        b.lastRead = level;
        b.lastChangeMs = millis();
    }
    if (millis() - b.lastChangeMs > DEBOUNCE_MS && level != b.stableLevel) {
        b.stableLevel = level;         // stable depuis 30 ms : changement confirmé
        return level == PRESSED_LEVEL; // vrai sur l'appui, faux sur le relâchement
    }
    return false;
}

void calibrateJoystick() {
    long sum = 0;
    for (int i = 0; i < 16; i++) {
        sum += analogRead(PIN_JOYSTICK_X);
        delay(2);
    }
    joystickCenterX = sum / 16;
}

// Convertit une lecture brute (0..4095) en valeur entre -1.0 et +1.0
float normalizeJoystick(int raw, int center) {
    int offset = raw - center;
    if (abs(offset) < JOYSTICK_DEADZONE) {
        return 0.0f;
    }
    int range = (offset > 0) ? (4095 - center) : center;
    return constrain((float)offset / range, -1.0f, 1.0f);
}

void setup() {
    Serial.begin(115200);
    pinMode(PIN_BUTTON, INPUT_PULLUP);
    calibrateJoystick();
}

void loop() {
    static int count = 0;
    if (justPressed(button)) {
        count++;
        Serial.printf("Appui n°%d\n", count);
    }
    Serial.printf("Joystick X: %.2f\n", normalizeJoystick(analogRead(PIN_JOYSTICK_X), joystickCenterX));
    delay(20);
}