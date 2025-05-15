// ControleMoteur.h
#ifndef CONTROLE_MOTEUR_H
#define CONTROLE_MOTEUR_H

#include <Arduino.h>
#include <Adafruit_MPU6050.h>
#include <Adafruit_Sensor.h>

class ControleMoteur
{
public:
    ControleMoteur(uint8_t in1, uint8_t in2, uint8_t in3, uint8_t in4);

    void begin();
    void attachIMU(Adafruit_MPU6050 &imu);
    void setTargetSpeeds(int16_t leftPct, int16_t rightPct);
    void setRampRate(float pctPerSec);
    void setFrictionAlpha(float alpha);
    void enableHeadingCorrection(bool enable);
    void calibrateFriction(uint32_t durationMs);
    float getLinearSpeed() const;
    void update();

private:
    uint8_t _in1, _in2, _in3, _in4;
    uint8_t _ch1, _ch2, _ch3, _ch4;
    Adafruit_MPU6050 *_imu;
    bool _hasIMU;
    float _targetL, _targetR;
    float _currentL, _currentR;
    float _rampRate;
    float _alphaFrot;
    float _frictionOffsetL, _frictionOffsetR;
    bool _headingCorr;
    float _heading0;
    float _kp, _ki, _kd;
    float _errInt, _lastErr;
    float _vel;
    float _accOffsetX;
    uint32_t _lastTime;

    void applyPWM(float lv, float rv);
    float computeYaw(float dt);
    void updateVelocity(float dt);
};

#endif // CONTROLE_MOTEUR_H