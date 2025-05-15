#pragma once
#include "Widget.h"
#include <functional>

class ButtonWidget : public Widget {
public:
  ButtonWidget(const String& name,
               uint32_t longMs    = 1000,
               uint32_t dblDelayMs= 300);

  void toInitJSON(JsonObject& o) override;
  void handleMessage(JsonObject& m) override;

  void onClick(std::function<void()> cb);
  void onDoubleClick(std::function<void()> cb);
  void onLongPress(std::function<void()> cb);

private:
  String _name;
  uint32_t _longMs;
  uint32_t _dblDelay;

  std::function<void()> _cbClick;
  std::function<void()> _cbDbl;
  std::function<void()> _cbLong;
};
