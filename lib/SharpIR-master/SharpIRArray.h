#ifndef SharpIRArray_h
#define SharpIRArray_h

#include <Adafruit_ADS1X15.h>

#define NB_SENSORS 3
#define NB_SAMPLE 1

class SharpIRArray
{
public:
    SharpIRArray(Adafruit_ADS1115 &ads);

    void update();                                   // Met à jour toutes les distances
    void begin();                                    // Initialise le capteur
    int getDistanceMM(int index);                    // Récupère la distance d’un capteur en mm
    void getAllDistances(int distances[NB_SENSORS]); // Remplit un tableau avec les distances

    bool getADSOk() { return _ads_ok; } // Getter pour l'état de l'ADS

private:
    Adafruit_ADS1115 &_ads;
    int _distances[NB_SENSORS];
    bool _ads_ok = true; // État de l'ADS

    int readSensorRaw(int channel) const;
    int computeDistanceMM(int raw) const;
};

#endif
