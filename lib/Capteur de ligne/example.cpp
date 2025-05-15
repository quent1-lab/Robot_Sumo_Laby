// Exemple d'utilisation (main.ino)
#include <Arduino.h>
#include "LineSensor.h"

// Déclaration des broches : 7 capteurs
const uint8_t pins[7] = {36, 26, 39, 25, 35, 33, 32};
LineSensor lineSensors(pins, 7);

void setup() {
  Serial.begin(115200);
  lineSensors.begin();

  // Calibration: balayer la surface pendant 3 secondes
  Serial.println("Demarrage calibration...");
  lineSensors.beginCalibration();
  uint32_t t0 = millis();
  while (millis() - t0 < 3000) {
    lineSensors.update();
  }
  lineSensors.endCalibration();
  Serial.println("Calibration terminee.");
}

void loop() {
  lineSensors.update();
  // Affichage des valeurs normalisees
  Serial.print("Line sensors: ");
  for (uint8_t i = 0; i < lineSensors.getSensorCount(); i++) {
    Serial.print(lineSensors.getValue(i), 3);
    Serial.print(i < lineSensors.getSensorCount()-1 ? ", " : "\n");
  }

  // Exemple: calcul de position du robot sur la ligne
  // (optionnel)
  delay(50);
}
