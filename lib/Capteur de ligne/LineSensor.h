// LineSensor.h
#ifndef LINE_SENSOR_H
#define LINE_SENSOR_H

#include <Arduino.h>

class LineSensor
{
public:
    // Constructeur
    // pins: tableau des numéros de broches, count: nombre de capteurs
    LineSensor(const uint8_t pins[], uint8_t count);

    // Initialisation, à appeler dans setup()
    void begin();

    // Démarre la phase de calibration
    // Pendant la calibration, appeler update() en balayant la surface
    void beginCalibration();

    // Termine la calibration et fixe les min/max calibrés
    void endCalibration();

    // Lit tous les capteurs, met à jour la calibration si en cours,
    // et calcule les valeurs linéarisées si en mode normal
    void update();

    // Accesseurs
    float getValue(uint8_t idx) const;    // valeur normalisée [0.0-1.0]
    int getRaw(uint8_t idx) const;        // valeur brute
    void getAllValues(float out[]) const; // récupère toutes les valeurs
    uint8_t getSensorCount() const;

private:
    uint8_t _pins[10]; // support max 10 capteurs
    uint8_t _count;

    int _raw[10];   // valeurs brutes
    float _val[10]; // valeurs linéaires normalisées

    int _minRaw[10]; // calibration: valeurs minimales
    int _maxRaw[10]; // calibration: valeurs maximales
    bool _calibrating;
};

#endif // LINE_SENSOR_H