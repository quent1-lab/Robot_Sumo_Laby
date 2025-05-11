#include "SharpIRArray.h"
#include <Arduino.h>
#include <math.h>

SharpIRArray::SharpIRArray(Adafruit_ADS1115* ads, bool filterEnabled[NB_SENSORS]) {
    _ads = ads;
    for (int i = 0; i < NB_SENSORS; ++i) {
        _useFilter[i] = filterEnabled[i];
        _distances[i] = 0;
    }
}

void SharpIRArray::update() {
    if (!_ads_ok) return; // Si l'ADS n'est pas ok, ne pas mettre à jour les distances
    // Met à jour toutes les distances
    for (int i = 0; i < NB_SENSORS; ++i) {
        _distances[i] = readSensor(i, _useFilter[i]);
    }
}

int SharpIRArray::getDistanceMM(int index) {
    if (index < 0 || index >= NB_SENSORS) return -1;
    return _distances[index];
}

void SharpIRArray::getAllDistances(int distances[NB_SENSORS]) {
    for (int i = 0; i < NB_SENSORS; ++i) {
        distances[i] = _distances[i];
    }
}

int SharpIRArray::readSensor(int channel, bool filter) {
    int samples[NB_SAMPLE];
    for (int i = 0; i < NB_SAMPLE; ++i) {
        samples[i] = _ads->readADC_SingleEnded(channel);
        delayMicroseconds(100);
    }

    int value;
    if (filter) {
        sort(samples, NB_SAMPLE);
        value = samples[NB_SAMPLE / 2];
    } else {
        int sum = 0;
        for (int i = 0; i < NB_SAMPLE; ++i) sum += samples[i];
        value = sum / NB_SAMPLE;
    }

    return computeDistanceMM(value);
}

int SharpIRArray::computeDistanceMM(int raw) {
    // Convertit lecture brute [0, 32767] en tension [0, 5000] mV puis en distance en mm
    float voltage = map(raw, 0, 32767, 0, 5000); // Convertit la lecture brute en mV
    voltage = constrain(voltage, 0.0, 5000.0); // Contrainte de la tension entre 0 et 5000 mV
    // Modèle 430 : distance en mm = 12080 * (tension en V)^-1.058
    float distanceMM = 120.8 * pow(voltage / 1000.0, -1.058); // modèle 430
    return static_cast<int>(distanceMM);
}

void SharpIRArray::sort(int arr[], int size) {
    for (int i = 0; i < size - 1; ++i) {
        for (int j = 0; j < size - i - 1; ++j) {
            if (arr[j] > arr[j + 1]) {
                int tmp = arr[j];
                arr[j] = arr[j + 1];
                arr[j + 1] = tmp;
            }
        }
    }
}