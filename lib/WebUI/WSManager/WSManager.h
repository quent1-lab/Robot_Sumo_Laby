#pragma once
#include <ArduinoJson.h>
#include <ESPAsyncWebServer.h>
#include <vector>
#include <functional>

class WSManager
{
public:
    using JsonCb = std::function<void(const JsonObject &)>;

    WSManager(AsyncWebSocket &ws);
    void begin();
    void broadcast(const JsonObject &obj);
    void onMessage(JsonCb cb);

private:
    AsyncWebSocket &_ws;
    std::vector<JsonCb> _callbacks;

    void _wsEvent(AsyncWebSocketClient *client,
                  AwsEventType type, void *arg,
                  uint8_t *data, size_t len);
};
