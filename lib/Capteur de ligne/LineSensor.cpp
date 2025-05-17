// LineSensor.cpp
#include "LineSensor.h"

LineSensor::LineSensor(const uint8_t pins[], uint8_t count)
    : _count(count), _calibrating(false)
{
    // On copie les broches
    for (uint8_t i = 0; i < _count; i++)
    {
        _pins[i] = pins[i];
        _minRaw[i] = 4095; // init min large (ADC 12-bit)
        _maxRaw[i] = 0;
    }
}

void LineSensor::begin()
{
    for (uint8_t i = 0; i < _count; i++)
    {
        pinMode(_pins[i], INPUT);
    }
}

void LineSensor::beginCalibration()
{
    _calibrating = true;
    // réinit des min/max
    for (uint8_t i = 0; i < _count; i++)
    {
        _minRaw[i] = 4095;
        _maxRaw[i] = 0;
    }
}

void LineSensor::endCalibration()
{
    _calibrating = false;
    // éviter division par zéro
    for (uint8_t i = 0; i < _count; i++)
    {
        if (_maxRaw[i] <= _minRaw[i] + 1)
        {
            // cas de surface uniforme -> on donne un petit pas
            _maxRaw[i] = _minRaw[i] + 1;
        }
        //Serial.printf("Capteur %d: min=%d, max=%d\n", i, _minRaw[i], _maxRaw[i]);
    }
}

void LineSensor::update()
{
    for (uint8_t i = 0; i < _count; i++)
    {
        int raw = analogRead(_pins[i]);
        _raw[i] = raw;

        if (_calibrating)
        {
            // mise à jour des min/max
            if (raw < _minRaw[i])
                _minRaw[i] = raw;
            if (raw > _maxRaw[i])
                _maxRaw[i] = raw;
            // pas de calcul de valeur normalisée
            _val[i] = 0.0f;
        }
        else
        {
            // linéarisation: map raw entre minRaw->maxRaw en [0.0-1.0]
            float v = float(raw - _minRaw[i]) / float(_maxRaw[i] - _minRaw[i]);
            if (v < 0.0f)
                v = 0.0f;
            if (v > 1.0f)
                v = 1.0f;
            _val[i] = v;
        }
    }
}

float LineSensor::getValue(uint8_t idx) const
{
    if (idx >= _count)
        return 0.0f;
    return _val[idx];
}

int LineSensor::getRaw(uint8_t idx) const
{
    if (idx >= _count)
        return 0;
    return _raw[idx];
}

void LineSensor::getAllValues(float out[]) const
{
    for (uint8_t i = 0; i < _count; i++)
    {
        out[i] = _val[i];
    }
}

uint8_t LineSensor::getSensorCount() const
{
    return _count;
}
// End of LineSensor.cpp