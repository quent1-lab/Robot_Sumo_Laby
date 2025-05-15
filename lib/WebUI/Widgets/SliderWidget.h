#pragma once
#include "Widget.h"

class SliderWidget : public Widget {
public:
    SliderWidget(const String& label,
                 const String& varName,
                 float minVal, float maxVal, float step);

    void toInitJSON(JsonObject& o) override;

private:
    String _label, _varName;
    float  _min_, _max_, _step;
};
