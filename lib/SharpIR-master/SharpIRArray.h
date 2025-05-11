#ifndef SharpIRArray_h
#define SharpIRArray_h

#include <Adafruit_ADS1X15.h>

#define NB_SENSORS 4
#define NB_SAMPLE 25

class SharpIRArray {
public:
    SharpIRArray(Adafruit_ADS1115* ads, bool filterEnabled[NB_SENSORS]);
    
    void update();  // Met à jour toutes les distances
    int getDistanceMM(int index);  // Récupère la distance d’un capteur en mm
    void getAllDistances(int distances[NB_SENSORS]);  // Remplit un tableau avec les distances

    void setADSOk(bool ok) { _ads_ok = ok; }  // Setter pour l'état de l'ADS
    bool getADSOk() { return _ads_ok; }  // Getter pour l'état de l'ADS

private:
    Adafruit_ADS1115* _ads;
    bool _useFilter[NB_SENSORS];
    int _distances[NB_SENSORS];
    bool _ads_ok = true;  // État de l'ADS

    int readSensor(int channel, bool filter);
    int computeDistanceMM(int raw);
    void sort(int arr[], int size);
};

#endif
