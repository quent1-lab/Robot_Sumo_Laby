#pragma once
#include "Widget.h"

class LabelWidget : public Widget {
public:
    LabelWidget(const String& label, const String& varName, const String& unit = {});
    void toInitJSON(JsonObject& o) override;

private:
    String _label, _varName, _unit;
};
