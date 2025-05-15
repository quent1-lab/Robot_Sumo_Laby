#pragma once
#include <Update.h>
#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>

class OTAHandler {
public:
  OTAHandler(AsyncWebServer &http);
  void begin();
private:
  AsyncWebServer &_http;
  void handleOtaPage(AsyncWebServerRequest* req);
  void handleOtaUpload(AsyncWebServerRequest* req,
                       const String& filename,
                       size_t index, uint8_t* data, size_t len, bool final);
};
