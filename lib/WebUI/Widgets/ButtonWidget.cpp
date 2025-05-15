#include "ButtonWidget.h"

ButtonWidget::ButtonWidget(const String& name,
                           uint32_t longMs,
                           uint32_t dblDelayMs)
  : _name(name), _longMs(longMs), _dblDelay(dblDelayMs)
{}

void ButtonWidget::toInitJSON(JsonObject& o) {
  o["widget"]    = "button";
  o["label"]     = _name;
  o["var"]       = _name;
  o["longPress"] = _longMs;
  o["dblDelay"]  = _dblDelay;
}

void ButtonWidget::handleMessage(JsonObject& m) {
  if (!m.containsKey("event")) return;
  String ev = m["event"].as<String>();
  if      (ev == "click"  && _cbClick) _cbClick();
  else if (ev == "double" && _cbDbl)   _cbDbl();
  else if (ev == "long"   && _cbLong)  _cbLong();
}

void ButtonWidget::onClick(std::function<void()> cb)       { _cbClick = cb; }
void ButtonWidget::onDoubleClick(std::function<void()> cb) { _cbDbl   = cb; }
void ButtonWidget::onLongPress(std::function<void()> cb)   { _cbLong  = cb; }
