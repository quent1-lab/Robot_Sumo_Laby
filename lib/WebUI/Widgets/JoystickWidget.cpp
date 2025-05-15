#include "JoystickWidget.h"

JoystickWidget::JoystickWidget(const String &name, float *x, float *y, float deadZone)
    : _xPtr(x), _yPtr(y), _deadZone(deadZone)
{
}

void JoystickWidget::setDeadZone(float dz)
{
    _deadZone = dz;
}

void JoystickWidget::toInitJSON(JsonObject &o)
{
    o["type"] = "joystick";
    o["name"] = _name;
    o["deadZone"] = _deadZone;
}

void JoystickWidget::notify(const JsonObject &msg)
{
    // Implémentez ici la logique de notification
    // Exemple : envoyer un message via WebSocket ou autre
    Serial.println("JoystickWidget notification:");
    serializeJson(msg, Serial);
    Serial.println();
}