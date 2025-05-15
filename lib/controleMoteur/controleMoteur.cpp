// ControleMoteur.cpp
#include "ControleMoteur.h"
#include <Wire.h>

#define ACC_SENSITIVITY 16384.0f
#define G_TO_MS2 9.80665f

ControleMoteur::ControleMoteur(uint8_t in1, uint8_t in2, uint8_t in3, uint8_t in4)
    : _in1(in1), _in2(in2), _in3(in3), _in4(in4),
      _ch1(0), _ch2(1), _ch3(2), _ch4(3),
      _imu(nullptr), _hasIMU(false),
      _targetL(0), _targetR(0), _currentL(0), _currentR(0),
      _rampRate(50.0f), _alphaFrot(0.1f),
      _frictionOffsetL(0), _frictionOffsetR(0),
      _headingCorr(false), _heading0(0),
      _kp(1.0f), _ki(0.0f), _kd(0.1f), _errInt(0), _lastErr(0),
      _vel(0), _accOffsetX(0), _lastTime(0)
{
}

void ControleMoteur::begin()
{
    ledcSetup(_ch1, 20000, 8);
    ledcSetup(_ch2, 20000, 8);
    ledcSetup(_ch3, 20000, 8);
    ledcSetup(_ch4, 20000, 8);
    ledcAttachPin(_in1, _ch1);
    ledcAttachPin(_in2, _ch2);
    ledcAttachPin(_in3, _ch3);
    ledcAttachPin(_in4, _ch4);
    pinMode(_in1, OUTPUT);
    pinMode(_in2, OUTPUT);
    pinMode(_in3, OUTPUT);
    pinMode(_in4, OUTPUT);

    _lastTime = millis();
    if (_hasIMU)
    {
        sensors_event_t a, g, temp;
        _imu->getEvent(&a, &g, &temp);
        _accOffsetX = a.acceleration.x;
        _heading0 = computeYaw(0);
    }
}

void ControleMoteur::attachIMU(Adafruit_MPU6050 &imu)
{
    _imu = &imu;
    _hasIMU = true;
}

void ControleMoteur::setTargetSpeeds(int16_t leftPct, int16_t rightPct)
{
    _targetL = constrain(leftPct, -100, 100);
    _targetR = constrain(rightPct, -100, 100);
}

void ControleMoteur::setRampRate(float pctPerSec) { _rampRate = max(1.0f, pctPerSec); }
void ControleMoteur::setFrictionAlpha(float a) { _alphaFrot = constrain(a, 0.0f, 1.0f); }
void ControleMoteur::enableHeadingCorrection(bool e) { _headingCorr = e; }

void ControleMoteur::calibrateFriction(uint32_t ms)
{
    uint32_t t0 = millis();
    float sumL = 0, sumR = 0;
    int n = 0;
    while (millis() - t0 < ms)
    {
        sumL += _currentL;
        sumR += _currentR;
        n++;
        delay(5);
    }
    _frictionOffsetL = sumL / n;
    _frictionOffsetR = sumR / n;
}

void ControleMoteur::update()
{
    uint32_t now = millis();
    float dt = (now - _lastTime) * 1e-3f;
    _lastTime = now;

    float corr = 0;
    if (_headingCorr && _hasIMU)
    {
        float yaw = computeYaw(dt);
        float e = yaw - _heading0;
        _errInt += e * dt;
        corr = _kp * e + _ki * _errInt + _kd * ((e - _lastErr) / dt);
        _lastErr = e;
    }

    auto ramp = [&](float &c, float t)
    {float d=t-c, m=_rampRate*dt; c+=constrain(d,-m,m); };
    ramp(_currentL, _targetL);
    ramp(_currentR, _targetR);

    float vL = _currentL * (1 + _alphaFrot) - _frictionOffsetL + corr;
    float vR = _currentR * (1 + _alphaFrot) - _frictionOffsetR - corr;

    applyPWM(vL, vR);

    if (_hasIMU)
        updateVelocity(dt);
}

void ControleMoteur::applyPWM(float lv, float rv)
{
    auto toP = [&](float v)
    { return uint8_t(255 * abs(constrain(v, -100, 100)) / 100.0f); };
    uint8_t pL = toP(lv), pR = toP(rv);
    if (lv >= 0)
    {
        ledcWrite(_ch1, pL);
        ledcWrite(_ch2, 0);
    }
    else
    {
        ledcWrite(_ch1, 0);
        ledcWrite(_ch2, pL);
    }
    if (rv >= 0)
    {
        ledcWrite(_ch3, pR);
        ledcWrite(_ch4, 0);
    }
    else
    {
        ledcWrite(_ch3, 0);
        ledcWrite(_ch4, pR);
    }
}

float ControleMoteur::computeYaw(float dt)
{
    sensors_event_t a, g, temp;
    _imu->getEvent(&a, &g, &temp);
    static float y = 0;
    y += g.gyro.z * dt;
    return y;
}

void ControleMoteur::updateVelocity(float dt)
{
    sensors_event_t a, g, temp;
    _imu->getEvent(&a, &g, &temp);
    float ax = (a.acceleration.x - _accOffsetX) * G_TO_MS2;
    _vel += ax * dt;
}

float ControleMoteur::getLinearSpeed() const { return _vel; }