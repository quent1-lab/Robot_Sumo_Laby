#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include "ecranOLED.h"

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 32
#define OLED_RESET -1
#define SCREEN_ADDRESS 0x3C

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

//constructor
ecranOLED::ecranOLED(){

}

void ecranOLED::begin() {
    // Initialisation de l'écran OLED
    if (!display.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS))
    {
        Serial.println(F("SSD1306 allocation failed"));
        while (true);
    }

    display.clearDisplay();
    display.display();
    afficherTexte("START");
}

bool ecranOLED::afficherTexte(String texte){
    display.clearDisplay();
    display.setCursor(10, 10);
    display.setTextSize(2);
    display.setTextColor(SSD1306_WHITE);

    display.setCursor(0, 0);
    display.print(texte);
    display.display();
    return true;
}