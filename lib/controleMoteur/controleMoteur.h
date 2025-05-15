// ControleMoteur.h
#ifndef CONTROLE_MOTEUR_H
#define CONTROLE_MOTEUR_H

#include <Arduino.h>
#include <Adafruit_MPU6050.h>
#include <Adafruit_Sensor.h>

class ControleMoteur
{
public:
    // Constructeur: pins IN1, IN2 for moteur Droit, IN3, IN4 pour moteur Gauche
    ControleMoteur(int in1, int in2, int in3, int in4);

    void attachMPU(Adafruit_MPU6050& mpu);

    // Initialise PWM, IMU, à appeler dans setup()
    void begin();

    // Définit le coefficient de frottement pour chaque moteur
    // (0..100% de la vitesse max)
    void setFriction(float fricD, float fricG);

    // Définit la consigne de vitesse [-100..100] pour chaque moteur
    void setSpeed(int speedD, int speedG);

    // Définit le taux de rampe d'accélération (%/s)
    void setRampRate(float percentPerSecond);

    // Active l'asservissement de cap
    void enableHeadingControl(bool enable);

    // Définit la consigne de cap (angle en degrés)
    void setHeadingTarget(float headingDeg);

    // Lance calibration du frottement pour les deux moteurs
    // Appuyer sur le bouton de la console (Serial) quand le moteur démarre
    void calibrateFriction();

    // Met à jour asservissements, rampes, IMU: à appeler en loop()
    void update();

    // Stoppe moteurs
    void stop();

private:
    // Broches et canaux PWM
    int _in1, _in2, _in3, _in4;
    int _ch1, _ch2, _ch3, _ch4;
    int _freq, _res;

    // Commandes
    float _targetD, _targetG;   // en %
    float _currentD, _currentG; // en %
    float _rampRate;            // % par seconde

    // Frottement
    float _fricThreshD, _fricThreshG; // seuil minimal pour mouvement

    // MPU
    Adafruit_MPU6050* _mpu;
    bool _hasMPU;
    bool _headingCtrl;
    float _headingTarget;
    float _heading;      // estimation
    float _kp, _ki, _kd; // PID gains
    float _iTerm, _lastError;

    // Horloge
    uint32_t _lastUpdate;

    // Fonctions internes
    void applyPWM(float cmdD, float cmdG);
    void readMPU(float dt);
};

#endif // CONTROLE_MOTEUR_H