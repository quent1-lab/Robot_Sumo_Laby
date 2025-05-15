// ControleMoteur.cpp
#include "ControleMoteur.h"

ControleMoteur::ControleMoteur(int in1, int in2, int in3, int in4)
    : _in1(in1), _in2(in2), _in3(in3), _in4(in4),
      _ch1(2), _ch2(3), _ch3(4), _ch4(5), _freq(20000), _res(8),
      _targetD(0), _targetG(0), _currentD(0), _currentG(0), _rampRate(100),
      _fricThreshD(0), _fricThreshG(0), _headingCtrl(false), _headingTarget(0), _heading(0),
      _kp(1.0), _ki(0.0), _kd(0.0), _iTerm(0), _lastError(0), _lastUpdate(0)
{
}

void ControleMoteur::begin()
{
    pinMode(_in1, OUTPUT);
    pinMode(_in2, OUTPUT);
    pinMode(_in3, OUTPUT);
    pinMode(_in4, OUTPUT);
    ledcSetup(_ch1, _freq, _res);
    ledcSetup(_ch2, _freq, _res);
    ledcSetup(_ch3, _freq, _res);
    ledcSetup(_ch4, _freq, _res);
    ledcAttachPin(_in1, _ch1);
    ledcAttachPin(_in2, _ch2);
    ledcAttachPin(_in3, _ch3);
    ledcAttachPin(_in4, _ch4);

    // Initialiser IMU
    if (!_mpu.begin())
    {
        Serial.println("IMU non trouvee");
    }
    _mpu.setAccelerometerRange(MPU6050_RANGE_4_G);
    _mpu.setGyroRange(MPU6050_RANGE_500_DEG);
    _mpu.setFilterBandwidth(MPU6050_BAND_21_HZ);

    _lastUpdate = millis();
}

void ControleMoteur::setSpeed(int speedD, int speedG)
{
    _targetD = constrain(speedD, -100, 100);
    _targetG = constrain(speedG, -100, 100);
}

void ControleMoteur::setRampRate(float percentPerSecond)
{
    _rampRate = max(percentPerSecond, 1.0f);
}

void ControleMoteur::enableHeadingControl(bool enable)
{
    _headingCtrl = enable;
    _iTerm = 0;
    _lastError = 0;
}

void ControleMoteur::setHeadingTarget(float headingDeg)
{
    _headingTarget = headingDeg;
}

void ControleMoteur::calibrateFriction()
{
    Serial.println("Calibration frottement moteur Droit: appuyez sur Entree quand bruit de roulement");
    for (float cmd = 0; cmd <= 100; cmd += 5)
    {
        applyPWM(cmd, 0);
        Serial.printf("Cmd Droit=%.0f\n", cmd);
        delay(500);
        if (Serial.available())
        {
            _fricThreshD = cmd;
            while (Serial.available())
                Serial.read();
            break;
        }
    }
    Serial.printf("Seuil frottement Droit=%.0f\n", _fricThreshD);

    Serial.println("Calibration frottement moteur Gauche: appuyez sur Entree");
    for (float cmd = 0; cmd <= 100; cmd += 5)
    {
        applyPWM(0, cmd);
        Serial.printf("Cmd Gauche=%.0f\n", cmd);
        delay(500);
        if (Serial.available())
        {
            _fricThreshG = cmd;
            while (Serial.available())
                Serial.read();
            break;
        }
    }
    Serial.printf("Seuil frottement Gauche=%.0f\n", _fricThreshG);

    stop();
}

void ControleMoteur::update()
{
    uint32_t now = millis();
    float dt = (now - _lastUpdate) / 1000.0f;
    _lastUpdate = now;

    // Mettre à jour estimation de cap
    readIMU(dt);

    // Appliquer asservissement de cap
    if (_headingCtrl)
    {
        float error = _headingTarget - _heading;
        _iTerm += _ki * error * dt;
        float dTerm = _kd * (error - _lastError) / dt;
        float corr = _kp * error + _iTerm + dTerm;
        _lastError = error;
        // corrige la consigne différentielle
        setSpeed(_targetD + corr, _targetG - corr);
    }

    // Appliquer rampe
    float rampStep = _rampRate * dt;
    if (abs(_currentD - _targetD) < rampStep)
        _currentD = _targetD;
    else
        _currentD += (_targetD > _currentD ? rampStep : -rampStep);
    if (abs(_currentG - _targetG) < rampStep)
        _currentG = _targetG;
    else
        _currentG += (_targetG > _currentG ? rampStep : -rampStep);

    // Appliquer seuil frottement
    float outD = (_currentD > 0 ? max(_currentD, _fricThreshD) : min(_currentD, -_fricThreshD));
    float outG = (_currentG > 0 ? max(_currentG, _fricThreshG) : min(_currentG, -_fricThreshG));

    applyPWM(outD, outG);
}

void ControleMoteur::stop()
{
    _targetD = _targetG = 0;
    _currentD = _currentG = 0;
    applyPWM(0, 0);
}

void ControleMoteur::applyPWM(float cmdD, float cmdG)
{
    // Convertir -100..100 en 0..255 PWM unipolaire et commandes sens
    auto pwmCalc = [&](float cmd, int chA, int chB)
    {
        int s = (cmd >= 0);
        uint8_t duty = uint8_t(min(abs(cmd) / 100.0f * 255.0f, 255.0f));
        if (s)
        {
            ledcWrite(chA, duty);
            ledcWrite(chB, 0);
        }
        else
        {
            ledcWrite(chA, 0);
            ledcWrite(chB, duty);
        }
    };
    pwmCalc(cmdD, _ch1, _ch2);
    pwmCalc(cmdG, _ch3, _ch4);
}

void ControleMoteur::readIMU(float dt)
{
    sensors_event_t a, g, temp;
    _mpu.getEvent(&a, &g, &temp);
    // Intégration gyroscopique sur Z pour cap
    _heading += g.gyro.z * dt * 180.0f / PI;
    // normaliser angle
    if (_heading > 180)
        _heading -= 360;
    if (_heading < -180)
        _heading += 360;
}
