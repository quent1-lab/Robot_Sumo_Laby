#pragma once
#include <ESPAsyncWebServer.h>

class HTTPServer
{
public:
    HTTPServer(AsyncWebServer &http);
    void begin();

private:
    AsyncWebServer &_http;

    const char *_indexHtml PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html lang="fr">
<head>
  <meta charset="UTF-8">
  <title>Robot WebUI</title>
  <style>
    /* Reset & variables */
    :root { --menu-width:260px; --gap:12px; --bg:#121212; --fg:#EEE; --card-bg:#1F1F1F; }
    * { box-sizing:border-box; margin:0; padding:0; }

    body {
      font-family:sans-serif;
      background:var(--bg);
      color:var(--fg);
      transition: margin-left .3s;
    }
    body.menu-open { margin-left: var(--menu-width); }

    /* Menu overlay */
    #menu {
      position:fixed; top:0; left:0;
      width: var(--menu-width); height:100%;
      background:#222; padding:10px;
      transform: translateX(-100%); transition: transform .3s;
      z-index:1000;
    }
    body.menu-open #menu { transform: translateX(0); }
    #menu h2 { color:#FFA; margin-bottom:8px; }
    #menu button {
      width:100%; margin:6px 0; padding:8px;
      border:none; background:#333; color:var(--fg);
      cursor:pointer;
    }
    #wifiForm, #otaForm { margin:8px 0; display:none; }
    #wifiForm input, #otaForm input[type=file] {
      width:100%; margin:4px 0; padding:6px;
      border:none; background:#333; color:var(--fg);
    }
    #otaForm button { width:100%; padding:8px; background:#0A84FF; color:#FFF; }

    /* Toggle bouton */
    #menuBtn {
      position:fixed; top:10px; left:10px;
      font-size:1.5em; color:var(--fg);
      cursor:pointer; z-index:1100;
    }

    /* Header */
    header {
      padding:16px; text-align:center;
      background:var(--card-bg);
      margin-bottom:var(--gap);
    }
    header h1 { font-size:1.2em; }

    /* Grille de tuiles */
    #container {
      display:grid;
      grid-gap: var(--gap);
      padding: var(--gap);
      /* on passe à auto-fit minmax pour responsive */
      grid-template-columns: repeat(auto-fit, minmax(200px, 1fr));
    }

    /* Carte générique */
    .card {
      background:var(--card-bg);
      border-radius:6px;
      padding: var(--gap);
      display:flex;
      flex-direction:column;
      align-items: stretch;
    }

    .card label {
      margin-bottom:6px;
      font-size:0.9em;
    }
    .card input[type=range] { width:100%; }
    .card input[type=number] {
      width: 60px; background:#222; color:var(--fg);
      border:none; padding:4px;
    }

    .btn {
      padding:8px; background:#0A84FF;
      border:none; color:#FFF; cursor:pointer;
      width:100%;
    }
    .label { font-size:1.1em; padding:6px; background:#333; flex:1; }

    /* Joystick */
    .joy-container { display:flex; justify-content:center; }
    #joy {
      width:120px; height:120px; background:var(--card-bg);
      border-radius:50%; position:relative; touch-action:none;
    }
    #stick {
      width:30px; height:30px; background:#0A84FF;
      border-radius:50%; position:absolute; top:45px; left:45px;
    }

    /* Logs */
    #logs {
      height:100px; overflow:auto;
      background:#222; padding:6px;
      font-size:0.85em; margin: var(--gap);
      border-radius:6px;
    }

    /* Media queries pour orientation */
    @media (orientation:portrait) {
      #container { grid-template-columns: 1fr; }
    }
    @media (orientation:landscape) {
      #container { grid-template-columns: repeat(auto-fit, minmax(250px, 1fr)); }
    }
  </style>
</head>
<body>
  <div id="menu">
    <h2>   Menu</h2>
    <button onclick="sendCmd('reset')">Reset Params</button>
    <button onclick="sendCmd('reboot')">Reboot ESP32</button>
    <button onclick="toggleWifiForm()">Wi‑Fi Mode</button>
    <button onclick="toggleOtaForm()">OTA Update</button>

    <div id="wifiForm">
      <input id="ssid"    placeholder="SSID">
      <input id="pwd" type="password" placeholder="Password">
      <button onclick="sendCmd('start_sta')">Connect STA</button>
      <button onclick="sendCmd('start_ap')">Start AP</button>
    </div>

    <div id="otaForm">
      <form id="frmOta" method="POST" action="/ota" enctype="multipart/form-data">
        <input type="file" name="firmware" accept=".bin">
        <button type="submit">Upload .bin</button>
      </form>
    </div>
  </div>

  <div id="menuBtn">☰</div>
  <header><h1>🤖 Robot Dashboard</h1></header>
  <div id="container"></div>
  <div id="logs"></div>

  <script>
    const ws = new WebSocket('ws://' + location.host + '/ws');
    const body = document.body;
    document.getElementById('menuBtn').onclick = () => {
      body.classList.toggle('menu-open');
    };
    function toggleWifiForm(){
      const f = document.getElementById('wifiForm');
      f.style.display = f.style.display==='block'?'none':'block';
    }
    function toggleOtaForm(){
      const f = document.getElementById('otaForm');
      f.style.display = f.style.display==='block'?'none':'block';
    }
    function sendCmd(cmd){
      ws.send(JSON.stringify({
        type:'cmd', cmd, 
        ssid:document.getElementById('ssid').value,
        pwd:document.getElementById('pwd').value
      }));
    }

    const container = document.getElementById('container');
    const logs = document.getElementById('logs');
    let joyActive=false;

    ws.onopen = ()=> log('WS connected');
    ws.onmessage = ev => {
      const m = JSON.parse(ev.data);

      if(m.type==='init'){
        // sliders
        m.params.forEach(p=>{
          const card = document.createElement('div');
          card.className='card';
          card.innerHTML = `
            <label for="r_${p.name}">${p.name}</label>
            <input type="range" id="r_${p.name}" min="${p.min}" max="${p.max}" step="0.01">
            <input type="number" id="n_${p.name}" min="${p.min}" max="${p.max}" step="0.01">
          `;
          container.appendChild(card);
          ['r_','n_'].forEach(pref=>{
            document.getElementById(pref+p.name)
              .addEventListener('input', e=>{
                const v=parseFloat(e.target.value);
                document.getElementById('r_'+p.name).value=v;
                document.getElementById('n_'+p.name).value=v;
                ws.send(JSON.stringify({type:'update',name:p.name,value:v}));
              });
          });
        });
        // buttons
        m.buttons.forEach(b=>{
          const card = document.createElement('div');
          card.className='card';
          const btn = document.createElement('button');
          btn.className='btn'; btn.textContent=b.name;
          btn.onclick = ()=> ws.send(JSON.stringify({type:'action',name:b.name}));
          card.appendChild(btn);
          container.appendChild(card);
        });
        // labels
        m.labels.forEach(l=>{
          const card = document.createElement('div');
          card.className='card';
          const lbl = document.createElement('div');
          lbl.className='label'; lbl.id='lbl_'+l.name; lbl.textContent=l.value;
          card.appendChild(lbl);
          container.appendChild(card);
        });
        // joystick
        if(m.joystick && !joyActive){
          joyActive=true;
          const card = document.createElement('div');
          card.className='card joy-container';
          card.innerHTML = `<div id="joy"><div id="stick"></div></div>`;
          container.appendChild(card);
          // JS de gestion du stick identique à ta version
        }
      }
      else if(m.type==='values'){
        m.params.forEach(p=>{
          document.getElementById('r_'+p.name).value=p.value;
          document.getElementById('n_'+p.name).value=p.value;
        });
      }
      else if(m.type==='labels'){
        m.labels.forEach(l=>{
          document.getElementById('lbl_'+l.name).textContent=l.value;
        });
      }
      else if(m.type==='log'){
        log(m.data);
      }
    };

    function log(txt){
      const d = document.createElement('div');
      d.textContent = '['+new Date().toLocaleTimeString()+'] '+txt;
      logs.appendChild(d);
      logs.scrollTop = logs.scrollHeight;
    }
  </script>
</body>
</html>
)rawliteral";
};
