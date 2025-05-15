#pragma once
#include "Widget.h"

/// Joystick virtuel (-1..1 sur x,y), définit dead-zone.
class JoystickWidget : public Widget
{
public:
    JoystickWidget(const String &name, float *x,float *y, float deadZone = 0.1f);

    void setDeadZone(float dz);

    void toInitJSON(JsonObject &o) override;
private:
    String _name;
    float _deadZone;
    float _x = 0, _y = 0;
    float *_xPtr, *_yPtr;
    void notify(const JsonObject &info);
};
