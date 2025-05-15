#pragma once
#include <WiFi.h>
#include <ESPmDNS.h>

class WiFiManager {
public:
  enum Mode { STA, AP };
  WiFiManager(const char* ssid, const char* pwd, const char* apName="WebUI-AP");
  bool beginSTA(unsigned long timeout=10000);
  void beginAP();
  String  localIP();
  String  hostName();

private:
  const char* _ssid;
  const char* _pwd;
  const char* _apName;
};
