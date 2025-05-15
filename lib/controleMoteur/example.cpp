#include <Wire.h>
#include <Adafruit_MPU6050.h>
#include "ControleMoteur.h"

Adafruit_MPU6050 imu;
ControleMoteur cm(2,3,4,5);

void setup() {
  Serial.begin(115200);
  Wire.begin();
  imu.begin();           // initialisation en main
  cm.attachIMU(imu);          // lien vers la lib
  cm.begin();

  cm.calibrateFriction(2000);
  cm.setRampRate(80);
  cm.setFrictionAlpha(0.1);
  cm.enableHeadingCorrection(true);
  cm.setTargetSpeeds(50,50);
}

void loop() {
  cm.update();
  // ...
}
