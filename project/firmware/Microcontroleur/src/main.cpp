#include <Spi.h>
#include <Adafruit_GFX.h>
#include <Adafruit_ST7789.h>
#include "config.h"

#define COULEUR(c) (~(c))

Adafruit_ST7789 tft(&SPI, PIN_TFT_CS, PIN_TFT_DC, PIN_TFT_RESET);

constexpr int PRESSED_LEVEL = LOW;           // niveau lu quand on appuie (étape 1)
constexpr unsigned long DEBOUNCE_MS = 30;

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

constexpr int JOYSTICK_DEADZONE = 300;
int joystickCenterX = 2048;

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

uint16_t colorBGR(uint16_t color) {
  return ((color & 0x001F) << 11) | (color & 0x07E0) | ((color & 0xF800) >> 11);
}
void setup() {
    Serial.begin(115200);
    delay(1000);

    pinMode(PIN_BUTTON, INPUT_PULLUP);
    delay(1000);

    pinMode(PIN_BUZZER, OUTPUT);
    delay(1000);
    // Test 1 : buzzer passif, une gamme do-ré-mi-fa-sol
    const int notes[] = {262, 294, 330, 349, 392};  // fréquences en Hz
    for (int note : notes) {
        tone(PIN_BUZZER, note, 200);
        delay(250);  // test de démarrage : attendre la fin de la note
    }
    delay(1000);

    // Test 2 : buzzer actif, un bip d'une demi-seconde
    digitalWrite(PIN_BUZZER, HIGH);
    delay(500);
    digitalWrite(PIN_BUZZER, LOW);
    delay(1000);

    calibrateJoystick();


    SPI.begin(PIN_TFT_SCK, -1, PIN_TFT_MOSI, PIN_TFT_CS); // pas de MISO
    tft.init(240, 320, SPI_MODE3);


    tft.setRotation(2); //portrait
    
    tft.invertDisplay(false);
    
    tft.fillScreen(ST77XX_WHITE);

    tft.setTextSize(4);
    tft.setTextColor(ST77XX_BLACK);
    tft.setCursor(72, 40);
    tft.print("PONG");

    tft.drawRect(20, 120, 200, 60, ST77XX_BLACK); // rectangle
    tft.fillCircle(120,250,20,ST77XX_YELLOW); // cercle jaune
}

void loop() {
    static int count = 0;
    if (justPressed(button)) {
        count++;
        Serial.printf("Appui n°%d\n", count);
    }

    float joystickX = normalizeJoystick(analogRead(PIN_JOYSTICK_X), joystickCenterX);

    if (joystickX != 0.0f) {
        Serial.printf("Joystick X: %.2f\n", joystickX);
    }
}