#include "OTAHandler.h"

OTAHandler::OTAHandler(AsyncWebServer &http): _http(http) {}

void OTAHandler::begin(){
  _http.on("/ota", HTTP_GET,  std::bind(&OTAHandler::handleOtaPage, this, std::placeholders::_1));
  _http.on("/ota", HTTP_POST,
    [](AsyncWebServerRequest* req){
      req->send(200,"text/plain","OK, rebooting");
      ESP.restart();
    },
    std::bind(&OTAHandler::handleOtaUpload, this,
              std::placeholders::_1,
              std::placeholders::_2,
              std::placeholders::_3,
              std::placeholders::_4,
              std::placeholders::_5,
              std::placeholders::_6)
  );
}

void OTAHandler::handleOtaPage(AsyncWebServerRequest* req){
  // on renvoie juste l’index html, car le form OTA est embarqué
  req->redirect("/");
}

void OTAHandler::handleOtaUpload(AsyncWebServerRequest* req,
                                 const String& filename,
                                 size_t index, uint8_t* data, size_t len, bool final){
  if(index==0){
    Update.begin(UPDATE_SIZE_UNKNOWN);
  }
  Update.write(data,len);
  if(final){
    Update.end(true);
  }
}
