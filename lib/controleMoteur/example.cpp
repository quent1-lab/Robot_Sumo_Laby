// Exemple d'utilisation (main.ino)
#include <Arduino.h>
#include "ControleMoteur.h"

ControleMoteur motors(14, 27, 26, 25);

void setup()
{
    Serial.begin(115200);
    motors.begin();

    // Calibration frottement
    delay(1000);
    motors.calibrateFriction();

    motors.setRampRate(200); // 200%/s
    motors.enableHeadingControl(true);
    motors.setHeadingTarget(0);
    motors.setSpeed(50, 50);
}

void loop()
{
    motors.update();
    // Ici on pourrait changer target ou heading
}
