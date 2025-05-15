#include "WiFiManager.h"

WiFiManager::WiFiManager(const char *ssid, const char *pwd, const char *apName)
    : _ssid(ssid), _pwd(pwd), _apName(apName) {}

bool WiFiManager::beginSTA(unsigned long timeout)
{
  Serial.printf("Connecting to WiFi %s", _ssid);
  WiFi.mode(WIFI_STA);
  WiFi.begin(_ssid, _pwd);
  unsigned long t0 = millis();
  while (WiFi.status() != WL_CONNECTED && millis() - t0 < timeout)
  {
    delay(200);
    Serial.print(".");
  }
  if (WiFi.status() == WL_CONNECTED)
  {
    MDNS.begin(_ssid);
    Serial.printf("Connected at %s\n", WiFi.localIP().toString().c_str());
    return true;
  }
  return false;
}

void WiFiManager::beginAP()
{
  WiFi.mode(WIFI_AP);
  WiFi.softAP(_apName);
  MDNS.begin(_apName);
}

String WiFiManager::localIP()
{
  return WiFi.localIP().toString();
}

String WiFiManager::hostName()
{
  return WiFi.getHostname();
}
