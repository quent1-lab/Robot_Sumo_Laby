#include <Arduino.h>
#include <Wire.h>

float linearizeSensor(int rawValue, int minValue, int maxValue);

void capteurLigne();

// Variable Capteur de ligne
const int nbrcapteurLigne = 7;
int PinCapteur[nbrcapteurLigne] = {36, 26, 39, 25, 35, 33, 32};
float ValeurCapteur[nbrcapteurLigne];

void setup(void)
{

  Serial.begin(115200);

  // Initialisation Capteur ligne
  for (int i = 0; i < nbrcapteurLigne; i++)
  {
    pinMode(PinCapteur[i], INPUT);
  }
}
void loop()
{
  capteurLigne();
}

void capteurLigne()
{

  for (int i = 0; i < nbrcapteurLigne; i++)
  {
    int pin = PinCapteur[i];

    if (pin == 25 || pin == 26)

    {
      ValeurCapteur[i] = float(digitalRead(PinCapteur[i]));
    }
    else
    {
      ValeurCapteur[i] = analogRead(PinCapteur[i]);
      // Linearize the sensor value to a range between 0.0 and 1.0
      ValeurCapteur[i] = linearizeSensor(ValeurCapteur[i], 0, 4095);
    }
    Serial.print(" LS");
    Serial.print(i + 1);
    Serial.print(" (pin ");
    Serial.print(PinCapteur[i]);
    Serial.print(") = ");
    Serial.println(ValeurCapteur[i]);
  }
}
float linearizeSensor(int rawValue, int minValue, int maxValue)
{
  // Map the raw sensor value to a range between 0.0 and 1.0
  float linearValue = (float)(rawValue - minValue) / (maxValue - minValue);
  // Clamp the value between 0.0 and 1.0
  if (linearValue < 0.0)
    linearValue = 0.0;
  if (linearValue > 1.0)
    linearValue = 1.0;
  return linearValue;
}