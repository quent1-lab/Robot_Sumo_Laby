#include "WSManager.h"

WSManager::WSManager(AsyncWebSocket &ws) : _ws(ws) {}

void WSManager::begin()
{
    // Capturer `this` via lambda pour conserver le contexte de l'instance
    _ws.onEvent([this](AsyncWebSocket *server, AsyncWebSocketClient *client,
                       AwsEventType type, void *arg, uint8_t *data, size_t len)
                { this->_wsEvent(client, type, arg, data, len); });
}

void WSManager::onMessage(JsonCb cb)
{
    _callbacks.push_back(cb);
}

void WSManager::_wsEvent(AsyncWebSocketClient *client,
                         AwsEventType type, void *arg,
                         uint8_t *data, size_t len)
{
    if (type != WS_EVT_DATA)
        return;

    AwsFrameInfo *info = static_cast<AwsFrameInfo *>(arg);
    if (!info->final || info->opcode != WS_TEXT)
        return;

    DynamicJsonDocument doc(512);
    DeserializationError err = deserializeJson(doc, (char *)data, len);
    if (err)
        return;

    JsonObject obj = doc.as<JsonObject>();

    for (auto &cb : _callbacks)
    {
        cb(obj);
    }
}

void WSManager::broadcast(const JsonObject &obj)
{
    String s;
    serializeJson(obj, s);
    _ws.textAll(s);
}
