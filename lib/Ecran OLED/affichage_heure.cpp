#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include "logo.h"
#include "bouton.h" // Inclusion de la bibliothèque bouton

// Définition des fonctions
void menuOption(int index);
void displayMenu();
void EcranD_acceuil();
void AcceuilOption(int index);
void Menu();
void afficherBatterie();

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 32
#define OLED_RESET -1
#define SCREEN_ADDRESS 0x3C

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

// ⚡ Définition des pins
#define BUTTON_NAVIGATE_PIN 0 // Bouton pour naviguer
#define BUTTON_SELECT_PIN 19  // Bouton pour sélectionner

// Création des objets pour les boutons
bouton boutonNavigate;
bouton boutonSelect;

int menuIndex = 0; // Index du menu (0 = Heure, 1 = Batterie, 2 = Réglages)

void setup()
{
    analogReadResolution(12); // Pour ESP32 avec 12 bits de résolution
    // Initialisation des boutons
    boutonNavigate.begin(BUTTON_NAVIGATE_PIN, false, 200, 500, 50); // false = bouton pull-up, avec délai de click et press
    boutonSelect.begin(BUTTON_SELECT_PIN, false, 200, 500, 50);     // idem pour le bouton Select

    Serial.begin(115200);

    if (!display.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS))
    {
        Serial.println(F("SSD1306 allocation failed"));
        while (true);
    }

    display.clearDisplay();
    display.display();
    EcranD_acceuil(); // Afficher l'écran d'accueil avec la batterie et le logo
}

void loop()
{
    boutonNavigate.read_Bt(); // Lire l'état du bouton "Naviguer"
    boutonSelect.read_Bt();   // Lire l'état du bouton "Sélectionner"

    // Navigation du menu avec boutonNavigate
    if (boutonNavigate.click()) // Si le bouton "Naviguer" a été cliqué
    {
        menuIndex = (menuIndex + 1) % 3; // Passe à l'option suivante (0 -> 1 -> 2 -> 0)
        displayMenu();
    }

    // Sélection du menu avec boutonSelect
    if (boutonSelect.click()) // Si le bouton "Sélectionner" a été cliqué
    {
        menuOption(menuIndex); // Exécute l'option en fonction de l'index du menu
    }
}

void Menu()
{
    if (boutonNavigate.click()) // Naviguer à l'option suivante
    {
        menuIndex = (menuIndex + 1) % 3; // Passe à l’option suivante
        displayMenu();
        delay(200); // Anti-rebond
    }

    if (boutonSelect.click()) // Sélectionner l'option
    {
        menuOption(menuIndex);
        delay(200); // Anti-rebond
    }
}

void displayMenu()
{
    display.clearDisplay();
    display.setTextSize(1);
    display.setTextColor(SSD1306_WHITE);

    display.setCursor(10, 5);
    display.println(menuIndex == 0 ? "> Heure" : "  Heure");

    display.setCursor(10, 15);
    display.println(menuIndex == 1 ? "> Batterie" : "  Batterie");

    display.setCursor(10, 25);
    display.println(menuIndex == 2 ? "> Reglages" : "  Reglages");

    display.display();
}

void menuOption(int index)
{
    display.clearDisplay();
    display.setCursor(10, 10);
    display.setTextSize(1);

    switch (index)
    {
    case 0:
        display.println("Affichage de l'heure...");
        break;
    case 1:
        display.println("Etat de la batterie...");
        afficherBatterie();
        break;
    case 2:
        display.println("Ouverture des reglages...");
        break;
    }

    display.display();
    delay(1000);
    displayMenu();
}

void AcceuilOption(int index)
{
    switch (index)
    {
    case 0:
        display.println("Menu");
        break;
    }
    display.display();
    Menu();
}

// Définition des constantes pour la batterie
#define R1 10000.0 // Résistance R1 en ohms
#define R2 10000.0 // Résistance R2 en ohms
#define V_BAT_MIN 6// Tension minimale de la batterie
#define V_BAT_MAX 8 // Tension maximale de la batterie

void EcranD_acceuil()
{
    float Vraw = analogRead(34); // Remplace par le bon pin de batterie si nécessaire
    float Vmesure = Vraw * (3.3 / 4095.0);
    float Vbatt = Vmesure * ((R1 + R2) / R1);
    float pourcentage = (Vbatt - V_BAT_MIN) / (V_BAT_MAX - V_BAT_MIN) * 100.0;
    pourcentage = constrain(pourcentage, 0.0, 100.0);

    Serial.print("Batterie: ");
    Serial.print(Vbatt, 2);
    Serial.print(" V (");
    Serial.print(pourcentage, 0);
    Serial.println("%)");

    // 🔋 Dessin de l'icône batterie
    display.drawRect(110, 5, 15, 7, SSD1306_WHITE);
    display.fillRect(125, 7, 2, 3, SSD1306_WHITE); // La pince

    int largeur = map(pourcentage, 0, 100, 0, 10);
    display.fillRect(112, 7, largeur, 3, SSD1306_WHITE); // Niveau

    // Texte "Menu"
    display.setTextSize(1);
    display.setTextColor(SSD1306_WHITE);
    display.setCursor(5, 20);
    display.println("Menu");

    // 🖼 Affichage logo
    display.drawBitmap(0, 0, bitmap, SCREEN_WIDTH, SCREEN_HEIGHT, WHITE);

    display.display();
}

// 🔋 Fonction dédiée si on veut rappeler le niveau batterie ailleurs
void afficherBatterie()
{
    float Vraw = analogRead(34);
    float Vmesure = Vraw * (3.3 / 4095.0);
    float Vbatt = Vmesure * ((R1 + R2) / R1);
    float pourcentage = (Vbatt - V_BAT_MIN) / (V_BAT_MAX - V_BAT_MIN) * 100.0;
    pourcentage = constrain(pourcentage, 0.0, 100.0);

    display.setCursor(10, 20);
    display.setTextSize(1);
    display.print("Vbat: ");
    display.print(Vbatt, 2);
    display.print("V  ");
    display.print(pourcentage, 0);
    display.println("%");
}