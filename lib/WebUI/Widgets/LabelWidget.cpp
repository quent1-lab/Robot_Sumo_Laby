#include "LabelWidget.h"

LabelWidget::LabelWidget(const String& label,
                         const String& varName,
                         const String& unit)
    : _label(label), _varName(varName), _unit(unit)
{}

void LabelWidget::toInitJSON(JsonObject& o) {
    o["widget"]  = "label";
    o["label"]   = _label;
    o["var"]     = _varName;
    if (_unit.length()) o["unit"] = _unit;
}
