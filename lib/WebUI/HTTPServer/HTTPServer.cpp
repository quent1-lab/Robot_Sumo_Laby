#include "HTTPServer.h"
#include <SPIFFS.h>

HTTPServer::HTTPServer(AsyncWebServer &http)
  : _http(http)
{}

void HTTPServer::begin(){
  // Monter SPIFFS
  if (!SPIFFS.begin(true)) {
    Serial.println("Failed to mount SPIFFS");
    return;
  }

  // Vérifiez si le fichier index.html existe
  const char* indexPath = "/index.html"; // Chemin relatif dans SPIFFS
  if (SPIFFS.exists(indexPath)) {
    Serial.println("index.html trouvé dans SPIFFS");
  } else {
    Serial.println("index.html introuvable dans SPIFFS");
  }

  // Servir index.html à la racine
  _http.on("/", HTTP_GET, [this](AsyncWebServerRequest* req) {
    const char* indexPath = "/data/index.html"; // Chemin relatif dans SPIFFS
    if (SPIFFS.exists(indexPath)) {
      req->send(SPIFFS, indexPath, "text/html");
    } else {
      req->send_P(200, "text/html", _indexHtml);
    }
  });

  // Lancer le serveur
  _http.begin();
}