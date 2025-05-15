// #pragma once
// #include <Arduino.h>
// #include <WiFi.h>
// #include <ESPAsyncWebServer.h>
// #include <SPIFFS.h>

// #include "WiFiManager/WiFiManager.h"
// #include "HTTPServer/HTTPServer.h"
// #include "WSManager/WSManager.h"
// #include "OTAHandler/OTAHandler.h"
// #include "Widgets/Widget.h"

// class WebUI
// {
// public:
//   WebUI(const char *ssid, const char *pwd, const char *hostname = "esp32");
//   ~WebUI();

//   // widgets génériques
//   void addWidget(Widget *widget);
//   void addSlider(const String &name, float *var, float minVal, float maxVal);
//   void addButton(const String &name);
//   void addLabel(const String &name, float *txtPtr, const String &unit = String());
//   void addJoystick(const String &name, float *x, float *y, float deadZone = 0.1f);
//   void addLog(const String &name, String *txtPtr);

//   void removeWidget(Widget *widget);

//   // à appeler dans setup()
//   void begin();
//   // à appeler dans loop()
//   void loop();

// private:
//   // Facades / modules
//   WiFiManager _wifi;
//   AsyncWebServer _http{80};
//   AsyncWebSocket _ws{"/ws"};
//   HTTPServer *_httpSrv;
//   WSManager *_wsMgr;
//   OTAHandler *_ota;

//   // liste de widgets
//   std::vector<Widget *> _widgets;

//   void onWsEvent(AsyncWebSocketClient *client, AwsEventType type,
//                  void *arg, uint8_t *data, size_t len);

//   // envoi init + updates
//   void broadcastInit(AsyncWebSocketClient *client = nullptr);
//   void broadcastUpdates();

//   // Reception
//   void handleMessage(const JsonObject &msg);

//}

#pragma once
#include <ArduinoJson.h>
#include <ESPAsyncWebServer.h>
#include <vector>
#include <functional>
#include <Widgets/Widget.h>

#include <WiFi.h>
#include <SPIFFS.h>

#include "WiFiManager/WiFiManager.h"
#include "HTTPServer/HTTPServer.h"
#include "WSManager/WSManager.h"
#include "OTAHandler/OTAHandler.h"
#include "Widgets/SliderWidget.h"
#include "Widgets/ButtonWidget.h"
#include "Widgets/LabelWidget.h"
#include "Widgets/JoystickWidget.h"

enum class VarType
{
  INT,
  FLOAT,
  BOOL,
  STRING
};

struct WebVar
{
  String name;
  VarType type;
  std::function<String()> getter;
  std::function<void(const String &)> setter;
};

class WebUI
{
public:
  WebUI(const char *ssid, const char *pwd, const char *hostname = nullptr);
  ~WebUI();

  // → Expose une variable générique
  void exposeVariable(
      const String &name,
      VarType type,
      std::function<String()> getter,
      std::function<void(const String &)> setter);

  // widgets
  void addWidget(Widget *w);
  void addSlider(const String &name, float *var, float minVal, float maxVal);
  void addButton(const String &name);
  void addLabel(const String &name, float *txtPtr, const String &unit = String());
  void addJoystick(const String &name, float *x, float *y, float deadZone = 0.1f);

  void begin();
  void loop(int16_t timeout);

private:
  WiFiManager _wifi;
  AsyncWebServer _http{80};
  AsyncWebSocket _ws{"/ws"};
  HTTPServer *_httpSrv;
  WSManager *_wsMgr;
  OTAHandler *_ota;

  // initialisation & mise à jour
  void broadcastInit(AsyncWebSocketClient *client = nullptr);
  void broadcastValues();

  // WebSocket handler
  void onWsEvent(AsyncWebSocketClient *, AwsEventType, void *, uint8_t *, size_t);

  // → recherche d’une variable dans le registre
  WebVar *findVar(const String &name);
  // …
  std::vector<WebVar> _vars;
  std::vector<Widget *> _widgets;

  const char *_indexHtml PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html lang="fr">
  <head>
    <meta charset="UTF-8" />
    <title>Robot WebUI</title>
    <style>
      /* Reset & variables */
      :root {
        --menu-width: 260px;
        --gap: 12px;
        --bg: #121212;
        --fg: #eee;
        --card-bg: #1f1f1f;
      }
      * {
        box-sizing: border-box;
        margin: 0;
        padding: 0;
      }

      body {
        font-family: sans-serif;
        background: var(--bg);
        color: var(--fg);
        transition: margin-left 0.3s;
      }
      body.menu-open {
        margin-left: var(--menu-width);
      }

      /* Menu overlay */
      #menu {
        position: fixed;
        top: 0;
        left: 0;
        width: var(--menu-width);
        height: 100%;
        background: #222;
        padding: 10px;
        transform: translateX(-100%);
        transition: transform 0.3s;
        z-index: 1000;
      }
      body.menu-open #menu {
        transform: translateX(0);
      }
      #menu h2 {
        color: #ffa;
        margin-bottom: 8px;
      }
      #menu button {
        width: 100%;
        margin: 6px 0;
        padding: 8px;
        border: none;
        background: #333;
        color: var(--fg);
        cursor: pointer;
      }
      #wifiForm,
      #otaForm {
        margin: 8px 0;
        display: none;
      }
      #wifiForm input,
      #otaForm input[type="file"] {
        width: 100%;
        margin: 4px 0;
        padding: 6px;
        border: none;
        background: #333;
        color: var(--fg);
      }
      #otaForm button {
        width: 100%;
        padding: 8px;
        background: #0a84ff;
        color: #fff;
      }

      /* Toggle bouton */
      #menuBtn {
        position: fixed;
        top: 10px;
        left: 10px;
        font-size: 1.5em;
        color: var(--fg);
        cursor: pointer;
        z-index: 1100;
      }

      /* Header */
      header {
        padding: 16px;
        text-align: center;
        background: var(--card-bg);
        margin-bottom: var(--gap);
      }
      header h1 {
        font-size: 1.2em;
      }

      /* Grille de tuiles */
      #container {
        display: grid;
        grid-gap: var(--gap);
        padding: var(--gap);
        /* on passe à auto-fit minmax pour responsive */
        grid-template-columns: repeat(auto-fit, minmax(200px, 1fr));
      }

      /* Carte générique */
      .card {
        background: var(--card-bg);
        border-radius: 6px;
        padding: var(--gap);
        display: flex;
        flex-direction: column;
        align-items: stretch;
      }

      .card label {
        margin-bottom: 6px;
        font-size: 0.9em;
      }
      .card input[type="range"] {
        width: 100%;
      }
      .card input[type="number"] {
        width: 60px;
        background: #222;
        color: var(--fg);
        border: none;
        padding: 4px;
      }

      .btn {
        padding: 8px;
        background: #0a84ff;
        border: none;
        color: #fff;
        cursor: pointer;
        width: 100%;
        transition: transform 0.1s ease; /* Transition pour l'animation */
      }

      .btn:active {
        transform: scale(0.95); /* Rétrécit légèrement le bouton lors du clic */
      }
      .btn:focus {
        outline: none; /* Supprime le contour de focus */
      }
      .label {
        font-size: 1.1em;
        padding: 6px;
        background: #333;
        flex: 1;
      }

      /* Joystick */
      .joy-container {
        display: flex;
        justify-content: center;
      }
      #joy {
        width: 120px;
        height: 120px;
        background: var(--card-bg);
        border-radius: 50%;
        position: relative;
        touch-action: none;
      }
      #stick {
        width: 30px;
        height: 30px;
        background: #0a84ff;
        border-radius: 50%;
        position: absolute;
        top: 45px;
        left: 45px;
      }

      /* Logs */
      #logs {
        height: 100px;
        overflow: auto;
        background: #222;
        padding: 6px;
        font-size: 0.85em;
        margin: var(--gap);
        border-radius: 6px;
      }

      /* Media queries pour orientation */
      @media (orientation: portrait) {
        #container {
          grid-template-columns: 1fr;
        }
      }
      @media (orientation: landscape) {
        #container {
          grid-template-columns: repeat(auto-fit, minmax(250px, 1fr));
        }
      }
    </style>
  </head>
  <body>
    <div id="menu">
      <h2 style="text-align: center">Menu</h2>
      <button onclick="sendCmd('reset')">Reset Params</button>
      <button onclick="sendCmd('reboot')">Reboot ESP32</button>
      <button onclick="toggleWifiForm()">Wi‑Fi Mode</button>
      <button onclick="toggleOtaForm()">OTA Update</button>

      <div id="wifiForm">
        <input id="ssid" placeholder="SSID" />
        <input id="pwd" type="password" placeholder="Password" />
        <button onclick="sendCmd('start_sta')">Connect STA</button>
        <button onclick="sendCmd('start_ap')">Start AP</button>
      </div>

      <div id="otaForm">
        <form
          id="frmOta"
          method="POST"
          action="/ota"
          enctype="multipart/form-data"
        >
          <input type="file" name="firmware" accept=".bin" />
          <button type="submit">Upload .bin</button>
        </form>
      </div>
    </div>

    <div id="menuBtn">☰</div>
    <header><h1>🤖 Robot Dashboard</h1></header>
    <div id="container"></div>
    <div id="logs"></div>

    <script>
      const ws = new WebSocket(`ws://${location.host}/ws`);
      const body = document.body;
      const menuBtn = document.getElementById("menuBtn");
      const container = document.getElementById("container");
      const logs = document.getElementById("logs");

      let joyActive = false;
      let joyState = { x: 0, y: 0 };

      // ─── Menu / Forms / Commandes ──────────────────────────────────────────
      menuBtn.addEventListener("click", () => {
        body.classList.toggle("menu-open");
      });
      function toggleWifiForm() {
        const f = document.getElementById("wifiForm");
        f.style.display = f.style.display === "block" ? "none" : "block";
      }
      function toggleOtaForm() {
        const f = document.getElementById("otaForm");
        f.style.display = f.style.display === "block" ? "none" : "block";
      }
      function sendCmd(cmd) {
        ws.send(
          JSON.stringify({
            type: "cmd",
            cmd,
            ssid: document.getElementById("ssid").value,
            pwd: document.getElementById("pwd").value,
          })
        );
      }

      // ─── WebSocket Events ──────────────────────────────────────────────────
      ws.onopen = () => log("WS connected");
      ws.onerror = (e) => log("WS error");
      ws.onclose = () => log("WS closed");
      ws.onmessage = (ev) => {
        let msg;
        try {
          msg = JSON.parse(ev.data);
        } catch (e) {
          console.error("WS: payload non JSON", ev.data);
          return;
        }
        //log("WS: " + JSON.stringify(msg));
        // INIT + initial values
        if (Array.isArray(msg.init)) {
          buildUI(msg.init);
          if (Array.isArray(msg.values)) {
            updateValues(msg.values);
          }
          return;
        }

        // subsequent updates
        if (Array.isArray(msg.values)) {
          updateValues(msg.values);
          return;
        }

        // logs, actions, joystick echo…
        if (msg.log) {
          log(msg.log);
          return;
        }
        if (msg.type === "action_ack") {
          console.info("Action ack:", msg);
          return;
        }
        if (msg.x != null && msg.y != null) {
          console.debug("Joystick echo:", msg);
          return;
        }
        if (msg.error) {
          console.warn("Server Error:", msg);
          return;
        }
        console.warn("WS: message non pris en charge", msg);
      };

      // ─── Construction de l'IHM ────────────────────────────────────────────
      function buildUI(cfgArr) {
        container.innerHTML = "";
        joyActive = false;
        cfgArr.forEach((cfg) => {
          log("Add Widget:", cfg.widget);
          switch (cfg.widget) {
            case "slider":
              createSlider(cfg);
              break;
            case "label":
              createLabel(cfg);
              break;
            case "button":
              createButton(cfg);
              break;
            case "joystick":
              createJoystick(cfg);
              break;
          }
        });
      }

      function createSlider(cfg) {
        const card = document.createElement("div");
        card.className = "card";
        card.innerHTML = `
      <label>${cfg.label}</label>
      <input type="range" data-var="${cfg.var}"
             min="${cfg.min}" max="${cfg.max}" step="${cfg.step}">
      <input type="number" data-var="${cfg.var}"
             min="${cfg.min}" max="${cfg.max}" step="${cfg.step}">
    `;
        container.appendChild(card);

        const debounceMap = {};

        function debounceSend(varName, value, delay = 100) {
          clearTimeout(debounceMap[varName]);
          debounceMap[varName] = setTimeout(() => {
            ws.send(JSON.stringify({
              type: "set",
              var: varName,
              value: value
            }));
          }, delay);
        }

        card.querySelectorAll("[data-var]").forEach(el => {
          el.value = cfg.value ?? cfg.min;
          el.addEventListener("input", e => {
            const v = parseFloat(e.target.value);
            card.querySelectorAll(`[data-var="${cfg.var}"]`)
                .forEach(x => x.value = v);
            debounceSend(cfg.var, v, 100); // 100 ms de délai
          });
        });
        }


      function createLabel(cfg) {
        const card = document.createElement("div");
        card.className = "card";
        const lbl = document.createElement("div");
        lbl.className = "label";
        lbl.dataset.var = cfg.var;
        lbl.textContent = cfg.value ?? "";
        if (cfg.unit) {
          lbl.textContent += ` ${cfg.unit}`;
        }
        card.appendChild(lbl);
        container.appendChild(card);
      }

      function createButton(cfg) {
      const card = document.createElement("div");
      card.className = "card";
      const btn = document.createElement("button");
      btn.className = "btn";
      btn.textContent = cfg.label;
      card.appendChild(btn);
      container.appendChild(card);

      // simple click
      btn.addEventListener("click", () => {
        ws.send(JSON.stringify({
          type:  "set",
          var:   cfg.var,
          event: "click"
        }));
      });

      // double click
      btn.addEventListener("dblclick", () => {
        ws.send(JSON.stringify({
          type:  "set",
          var:   cfg.var,
          event: "double"
        }));
      });

      // long press detection
      let pressTimer = null;
      btn.addEventListener("pointerdown", () => {
        pressTimer = setTimeout(() => {
          ws.send(JSON.stringify({
            type:  "set",
            var:   cfg.var,
            event: "long"
          }));
        }, cfg.longPress);
      });
      btn.addEventListener("pointerup", () => {
        clearTimeout(pressTimer);
      });
    }


      function createJoystick(cfg) {
        if (joyActive) return;
        joyActive = true;
        const card = document.createElement("div");
        card.className = "card joy-container";
        card.innerHTML = `<div id="joy"><div id="stick"></div></div>`;
        container.appendChild(card);
        initJoystick(cfg.deadZone);
      }

      // ─── Mise à jour universelle ───────────────────────────────────────────
      function updateValues(vals) {
        vals.forEach((p) => {
          // range & number inputs
          document
            .querySelectorAll(`[data-var="${p.var}"]`)
            .forEach((el) => (el.value = p.value));
          // labels
          const lbl = document.querySelector(`.label[data-var="${p.var}"]`);
          if (lbl) lbl.textContent = p.value + (p.unit ? ` ${p.unit}` : "");
        });
      }

      // ─── Log interne ───────────────────────────────────────────────────────
      function log(txt) {
        const d = document.createElement("div");
        d.textContent = `[${new Date().toLocaleTimeString()}] ${txt}`;
        logs.appendChild(d);
        logs.scrollTop = logs.scrollHeight;
      }

      // ─── Joystick ──────────────────────────────────────────────────────────
      function initJoystick(deadZone = 0.1) {
        const joy = document.getElementById("joy");
        const stick = document.getElementById("stick");
        if (!joy || !stick) return;

        let rect = joy.getBoundingClientRect();
        const cx = rect.width / 2,
          cy = rect.height / 2;
        const r = rect.width / 2 - stick.offsetWidth / 2;
        let state = { x: 0, y: 0 };

        function update(x, y) {
          let d = Math.hypot(x, y);
          if (d > r) {
            x *= r / d;
            y *= r / d;
          }
          stick.style.left = `${x + cx - stick.offsetWidth / 2}px`;
          stick.style.top = `${y + cy - stick.offsetHeight / 2}px`;

          let nx = +(x / r).toFixed(2),
            ny = +(-y / r).toFixed(2);
          if (Math.abs(nx) < deadZone) nx = 0;
          if (Math.abs(ny) < deadZone) ny = 0;

          if (nx !== state.x || ny !== state.y) {
            state = { x: nx, y: ny };
            ws.send(JSON.stringify({ type: "joy", x: nx, y: ny }));
          }
        }

        function move(e) {
          const x = e.clientX - rect.left - cx;
          const y = e.clientY - rect.top - cy;
          update(x, y);
        }

        joy.addEventListener("pointerdown", (e) => {
          rect = joy.getBoundingClientRect();
          joy.setPointerCapture(e.pointerId);
          move(e);
        });
        joy.addEventListener("pointermove", (e) => {
          if (e.pressure > 0) move(e);
        });
        joy.addEventListener("pointerup", (e) => {
          joy.releasePointerCapture(e.pointerId);
          update(0, 0);
        });
      }
    </script>
  </body>
</html>

)rawliteral";
};