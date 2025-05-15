#include "SharpIRArray.h"
#include <Arduino.h>
#include <math.h>

SharpIRArray::SharpIRArray()
{
    for (int i = 0; i < NB_SENSORS; i++)
    {
        _distances[i] = 0;
    }
}

void SharpIRArray::begin()
{
    // Initialisation I2C + ADS
    if (!_ads.begin())
    {
        Serial.println("Failed to initialize ADS.");
        _ads_ok = false;
    }
    else
    {
        Serial.println("ADS initialized.");

        // Mode conversion continue, gain = ±4.096V, DataRate = 860SPS (max)
        _ads.setGain(GAIN_ONE); // ±4.096V
        _ads.setDataRate(RATE_ADS1115_860SPS); // Vitesse maximale
        _ads.startComparator_SingleEnded(0, 0x7FFF);
        // on force la conversion continue
        // for (int ch = 0; ch < NB_SENSORS; ch++)
        // {
        //     _ads.startContinuousConversion(ch); // Removed as Adafruit_ADS1115 does not support this method
        // }
    }
}

void SharpIRArray::update()
{
    if (!_ads_ok)
        return; // Si l'ADS n'est pas ok, ne pas mettre à jour les distances
                // On lit chacun des canaux
    for (int i = 0; i < NB_SENSORS; i++)
    {
        int raw = readSensorRaw(i);
        _distances[i] = computeDistanceMM(raw);
    }
}

int SharpIRArray::getDistanceMM(int index)
{
    if (index < 0 || index >= NB_SENSORS)
        return -1;
    return _distances[index];
}

void SharpIRArray::getAllDistances(int distances[NB_SENSORS])
{
    for (int i = 0; i < NB_SENSORS; ++i)
    {
        distances[i] = _distances[i];
    }
}

int SharpIRArray::readSensorRaw(int channel)
{
    // Lit directement la dernière conversion continue
    return _ads.readADC_SingleEnded(channel);
}

int SharpIRArray::computeDistanceMM(int raw) const
{
    // Convertit lecture brute [0, 32767] en tension [0, 5000] mV puis en distance en mm
    float voltage = map(raw, 0, 32767, 0, 5000); // Convertit la lecture brute en mV
    voltage = constrain(voltage, 0.0, 5000.0);   // Contrainte de la tension entre 0 et 5000 mV
    // Modèle 430 : distance en mm = 12080 * (tension en V)^-1.058
    float distanceMM = 120.8 * pow(voltage / 1000.0, -1.058); // modèle 430
    return static_cast<int>(distanceMM);
}

// void SharpIRArray::sort(int arr[], int size)
// {
//     for (int i = 0; i < size - 1; ++i)
//     {
//         for (int j = 0; j < size - i - 1; ++j)
//         {
//             if (arr[j] > arr[j + 1])
//             {
//                 int tmp = arr[j];
//                 arr[j] = arr[j + 1];
//                 arr[j + 1] = tmp;
//             }
//         }
//     }
// }