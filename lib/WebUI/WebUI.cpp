// #include "WebUI.h"
// #include <SPIFFS.h>
// #include <ESPmDNS.h>

// WebUI::WebUI(const char *ssid, const char *pwd, const char *hostname)
//     : _ssid(ssid), _pwd(pwd), _hostname(hostname)
// {
// }

// WebUI::~WebUI()
// {
//     _ws.cleanupClients();
// }

// // Ajout d’objets
// void WebUI::addParam(const String &name, float *var, float minVal, float maxVal)
// {
//     _params.push_back({name, var, minVal, maxVal});
// }
// void WebUI::addButton(const String &name)
// {
//     _buttons.push_back({name});
// }
// void WebUI::addLabel(const String &name, String *txtPtr)
// {
//     _labels.push_back({name, txtPtr});
// }

// void WebUI::begin()
// {
//     // SPIFFS
//     if (!SPIFFS.begin(true))
//     {
//         Serial.println("SPIFFS failed");
//     }
//     // Wi-Fi station
//     WiFi.mode(WIFI_STA);
//     WiFi.begin(_ssid, _pwd);
//     while (WiFi.status() != WL_CONNECTED)
//         delay(200);
//     MDNS.begin(_hostname);

//     // WebSocket
//     _ws.onEvent([this](AsyncWebSocket *, AsyncWebSocketClient *c,
//                        AwsEventType t, void *arg, uint8_t *d, size_t l)
//                 { this->onWsEvent(c, t, arg, d, l); });
//     _http.addHandler(&_ws);

//     // Routes HTTP
//     _http.on("/", HTTP_GET, [&](AsyncWebServerRequest *r)
//              { handleRoot(r); });

//     // OTA form handler (serve page)
//     _http.on("/ota", HTTP_GET, [&](AsyncWebServerRequest *r)
//              {
//     // La page OTA est incluse dans le menu, on redirige
//     r->redirect("/"); });

//     // OTA upload handler
//     _http.on("/ota", HTTP_POST, [&](AsyncWebServerRequest *r)
//              {
//       // fin de l’upload
//       r->send(200, "text/plain", "OK, rebooting");
//       ESP.restart(); }, [&](AsyncWebServerRequest *r, const String &filename, size_t index, uint8_t *data, size_t len, bool final)
//              {
//       if(index==0){
//         Serial.println("OTA start");
//         if(!Update.begin(UPDATE_SIZE_UNKNOWN)){
//           Update.printError(Serial);
//           return;
//         }
//       }
//       if(Update.write(data,len)!=len){
//         Update.printError(Serial);
//         return;
//       }
//       if(final){
//         if(!Update.end(true)){
//           Update.printError(Serial);
//         } else {
//           Serial.println("OTA done");
//         }
//       } });

//     _http.begin();
//     // Envoi initial
//     broadcastInit();
// }

// void WebUI::loop()
// {
//     static unsigned long last = 0;
//     if (millis() - last > 500)
//     {
//         last = millis();
//         broadcastValues();
//         broadcastLabels();
//     }
// }

// // Commandes menu
// WebUI::Cmd WebUI::fetchCommand() { return _pendingCmd; }
// void WebUI::clearCommand() { _pendingCmd = CMD_NONE; }

// // Handle page racine
// void WebUI::handleRoot(AsyncWebServerRequest *req)
// {
//     req->send_P(200, "text/html", _indexHtml);
// }

// // WS events
// void WebUI::onWsEvent(AsyncWebSocketClient *client, AwsEventType type,
//                       void *arg, uint8_t *data, size_t len)
// {
//     if (type == WS_EVT_CONNECT)
//     {
//         broadcastInit(client);
//     }
//     else if (type == WS_EVT_DATA)
//     {
//         AwsFrameInfo *info = static_cast<AwsFrameInfo *>(arg);
//         if (info->final && info->opcode == WS_TEXT)
//         {
//             String msg((char *)data);
//             DynamicJsonDocument doc(512);
//             deserializeJson(doc, msg);
//             String t = doc["type"].as<String>();
//             if (t == "update")
//             {
//                 handleUpdate(doc["name"].as<String>(), doc["value"].as<float>());
//             }
//             else if (t == "action")
//             {
//                 handleAction(doc["name"].as<String>());
//             }
//             else if (t == "cmd")
//             {
//                 handleCmd(doc["cmd"].as<String>(),
//                           doc["ssid"].as<String>(),
//                           doc["pwd"].as<String>());
//             }
//         }
//     }
// }

// // Envoi init
// void WebUI::broadcastInit(AsyncWebSocketClient *client)
// {
//     DynamicJsonDocument doc(1024);
//     doc["type"] = "init";
//     auto a1 = doc.createNestedArray("params");
//     for (auto &p : _params)
//     {
//         auto o = a1.createNestedObject();
//         o["name"] = p.name;
//         o["min"] = p.minVal;
//         o["max"] = p.maxVal;
//         o["value"] = *(p.ptr);
//     }
//     auto a2 = doc.createNestedArray("buttons");
//     for (auto &b : _buttons)
//     {
//         auto o = a2.createNestedObject();
//         o["name"] = b.name;
//     }
//     auto a3 = doc.createNestedArray("labels");
//     for (auto &l : _labels)
//     {
//         auto o = a3.createNestedObject();
//         o["name"] = l.name;
//         o["value"] = *(l.ptr);
//     }
//     // joystick déja géré côté HTML
//     String s;
//     serializeJson(doc, s);
//     if (client)
//         client->text(s);
//     else
//         _ws.textAll(s);
// }

// // Envoi valeurs sliders
// void WebUI::broadcastValues()
// {
//     DynamicJsonDocument doc(512);
//     doc["type"] = "values";
//     auto a = doc.createNestedArray("params");
//     for (auto &p : _params)
//     {
//         auto o = a.createNestedObject();
//         o["name"] = p.name;
//         o["value"] = *(p.ptr);
//     }
//     String s;
//     serializeJson(doc, s);
//     _ws.textAll(s);
// }

// // Envoi labels
// void WebUI::broadcastLabels()
// {
//     DynamicJsonDocument doc(512);
//     doc["type"] = "labels";
//     auto a = doc.createNestedArray("labels");
//     for (auto &l : _labels)
//     {
//         auto o = a.createNestedObject();
//         o["name"] = l.name;
//         o["value"] = *(l.ptr);
//     }
//     String s;
//     serializeJson(doc, s);
//     _ws.textAll(s);
// }

// // Réceptions
// void WebUI::handleUpdate(const String &name, float value)
// {
//     for (auto &p : _params)
//     {
//         if (p.name == name)
//             *(p.ptr) = constrain(value, p.minVal, p.maxVal);
//     }
// }

// void WebUI::handleAction(const String &name)
// {
//     // À personnaliser si besoin
//     Serial.printf("Action: %s\n", name.c_str());
// }

// void WebUI::handleCmd(const String &cmd,
//                       const String &ssid, const String &pwd)
// {
//     if (cmd == "reset")
//     {
//         _pendingCmd = CMD_RESET;
//     }
//     else if (cmd == "reboot")
//     {
//         _pendingCmd = CMD_REBOOT;
//     }
//     else if (cmd == "start_ap")
//     {
//         _pendingCmd = CMD_START_AP;
//     }
//     else if (cmd == "start_sta")
//     {
//         WiFi.begin(ssid.c_str(), pwd.c_str());
//         _pendingCmd = CMD_START_STA;
//     }
// }

#include "WebUI.h"
#include <ArduinoJson.h>

WebUI::WebUI(const char *ssid, const char *pwd, const char *hostname)
    : _wifi(ssid, pwd, hostname)
{
}

WebUI::~WebUI()
{
    delete _httpSrv;
    delete _wsMgr;
    delete _ota;
    for (auto *w : _widgets)
        delete w;
}

void WebUI::addWidget(Widget *widget)
{
    _widgets.push_back(widget);
}

void WebUI::begin()
{
    // 1) SPIFFS
    SPIFFS.begin(true);

    // 2) Wi‑Fi
    if (!_wifi.beginSTA())
        _wifi.beginAP();

    // 3) Route racine → indexHtml
    _http.on("/", HTTP_GET, [&](AsyncWebServerRequest *req)
             { req->send_P(200, "text/html", _indexHtml); });

    // 4) OTA
    _ota = new OTAHandler(_http);
    _ota->begin();

    // 5) Attacher le WS au serveur HTTP
    _http.addHandler(&_ws);

    // 6) Configurer onEvent AVANT http.begin()
    _ws.onEvent([this](AsyncWebSocket *, AsyncWebSocketClient *client,
                       AwsEventType type, void *arg, uint8_t *data, size_t len)
                { this->onWsEvent(client, type, arg, data, len); });

    // 7) Démarrer HTTP & WS
    _http.begin();

    // 8) Envoyer INIT
    // broadcastInit();
}

void WebUI::loop(int16_t timeout)
{
    static unsigned long last = 0;
    if (millis() - last < timeout)
        return;
    last = millis();

    // Si personne n’est connecté, pas la peine d’envoyer
    if (_ws.count() > 0)
    {
        broadcastValues();
    }
}

void WebUI::exposeVariable(
    const String &name,
    VarType type,
    std::function<String()> getter,
    std::function<void(const String &)> setter)
{
    _vars.push_back({name, type, getter, setter});
}

WebVar *WebUI::findVar(const String &name)
{
    for (auto &v : _vars)
    {
        if (v.name == name)
            return &v;
    }
    return nullptr;
}

void WebUI::broadcastInit(AsyncWebSocketClient *client)
{
    DynamicJsonDocument doc(4096);
    auto arr = doc.createNestedArray("init");

    // 1) descriptors de widgets
    for (auto *w : _widgets)
    {
        JsonObject o = arr.createNestedObject();
        w->toInitJSON(o);
    }

    // 2) valeurs initiales
    auto vals = doc.createNestedArray("values");
    for (auto &v : _vars)
    {
        JsonObject o = vals.createNestedObject();
        o["var"] = v.name;
        String s = v.getter();
        switch (v.type)
        {
        case VarType::INT:
            o["value"] = s.toInt();
            break;
        case VarType::FLOAT:
            o["value"] = s.toFloat();
            break;
        case VarType::BOOL:
            o["value"] = (s == "1");
            break;
        case VarType::STRING:
            o["value"] = s;
            break;
        }
    }

    String out;
    serializeJson(doc, out);
    if (client)
        _ws.text(client->id(), out);
    else
        _ws.textAll(out);
}

void WebUI::broadcastValues()
{
    if (_ws.count() == 0)
        return;

    DynamicJsonDocument doc(2048);
    auto arr = doc.createNestedArray("values");
    for (auto &v : _vars)
    {
        JsonObject o = arr.createNestedObject();
        o["var"] = v.name;
        String s = v.getter();
        switch (v.type)
        {
        case VarType::INT:
            o["value"] = s.toInt();
            break;
        case VarType::FLOAT:
            o["value"] = s.toFloat();
            break;
        case VarType::BOOL:
            o["value"] = (s == "1");
            break;
        case VarType::STRING:
            o["value"] = s;
            break;
        }
    }
    String out;
    serializeJson(doc, out);
    _ws.textAll(out);
}

void WebUI::onWsEvent(AsyncWebSocketClient *client,
                      AwsEventType type, void *arg,
                      uint8_t *data, size_t len)
{
    if (type == WS_EVT_CONNECT)
    {
        broadcastInit(client);
    }
    else if (type != WS_EVT_DATA)
        return;
    AwsFrameInfo *info = (AwsFrameInfo *)arg;
    if (!info->final || info->opcode != WS_TEXT)
        return;

    DynamicJsonDocument doc(512);
    if (deserializeJson(doc, data, len).code() != DeserializationError::Ok)
        return;
    auto root = doc.as<JsonObject>();
    String t = root["type"].as<String>();

    if (t == "set")
    {
        for (auto *w : _widgets)
        {
            w->handleMessage(root);
        }
        String vn = root["var"].as<String>();
        String vv = root["value"].as<String>();
        if (auto *v = findVar(vn))
        {
            v->setter(vv);
            broadcastValues();
        }
    }
    else if (t == "getAll")
    {
        broadcastInit(client);
    }
    // … tu peux ajouter get(var) si besoin
}
