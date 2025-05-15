#include "SliderWidget.h"

SliderWidget::SliderWidget(const String& label,
                           const String& varName,
                           float minVal, float maxVal, float step)
    : _label(label), _varName(varName),
      _min_(minVal), _max_(maxVal), _step(step)
{}

void SliderWidget::toInitJSON(JsonObject& o) {
    o["widget"] = "slider";
    o["label"]  = _label;
    o["var"]    = _varName;
    o["min"]    = _min_;
    o["max"]    = _max_;
    o["step"]   = _step;
}
