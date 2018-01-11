#include "ConfigServer.h"
#include <ESP8266mDNS.h>
#include "index.html.h"

void doUpload(AsyncWebServerRequest *request, const String& filename, size_t index, uint8_t *data, size_t len, bool final) {
  if(final) {
    Serial.printf("Uploaded: %s\n", filename.c_str());
  }    
}

void ConfigServer::setup(AsyncWebServer *server) {
  server->on("/config", HTTP_GET, [](AsyncWebServerRequest *request) {
    AsyncWebServerResponse *response = request->beginResponse_P(200, "text/html; charset=utf-8", index_html_gz, index_html_gz_len);
    response->addHeader("Content-Encoding", "gzip");
    request->send(response);
  });

  WiFi.scanNetworks();
  server->on("/aps.json", HTTP_GET, [](AsyncWebServerRequest *request) {
    int num = WiFi.scanComplete();
   
    if(num == -2){
      num = WiFi.scanNetworks(true);
    }

    int indices[num];
    for(int i = 0; i < num; i++) {
      indices[i] = i;
    }

    // Bubble Sort - Strongest signal first.
    bool completed = false;
    while(!completed) {
      completed = true; 
      for(int i = 0; i < num - 1; i++) {
        if(WiFi.RSSI(indices[i]) < WiFi.RSSI(indices[i + 1])) {
          int temp = indices[i];
          indices[i] = indices[i + 1];
          indices[i + 1] = temp;
          completed = false;
        }
      }
    }

    String json = "[";
    if(num > 0) {
      bool start = true;
      for(int i = 0; i < num; i++) {
        if(!start) {
          json += ",";
        } else {
          start = false;
        }
        
        json += "{";
        json += "\"ssid\":\"" + WiFi.SSID(indices[i])+"\"";
        json += ",\"rssi\":" + String(WiFi.RSSI(indices[i]));
        json += ",\"encryption\":" + String(WiFi.encryptionType(indices[i]));
        json += "}";
      }
      
      WiFi.scanDelete();
      
      if(WiFi.scanComplete() == -2){
        WiFi.scanNetworks(true);
      }
    }
    json += "]";
        
    AsyncWebServerResponse *response = request->beginResponse(200,"application/json", json);
    response->addHeader("Cache-Control", "no-cache, no-store, must-revalidate");
    response->addHeader("Pragma", "no-cache");
    response->addHeader("Expires", "-1");
    request->send(response);
  });

  server->on("/config/update", HTTP_POST, [](AsyncWebServerRequest *request) {
    int params = request->params();
    for(int i=0;i<params;i++){
      AsyncWebParameter* p = request->getParam(i);
      if(p->isFile()){ //p->isPost() is also true
        Serial.printf("FILE[%s]: %s, size: %u\n", p->name().c_str(), p->value().c_str(), p->size());
      } else if(p->isPost()){
        Serial.printf("POST[%s]: %s\n", p->name().c_str(), p->value().c_str());
      } else {
        Serial.printf("GET[%s]: %s\n", p->name().c_str(), p->value().c_str());
      }
    }
    if(request->hasParam("config.dat", true)) {
      AsyncWebParameter* p = request->getParam("config.dat", true);
      Serial.printf("config: %s\n", p->value().c_str());
    }
    request->send(200);
  }, doUpload);
  
  server->serveStatic("/config.dat", SPIFFS, "/config.dat");

  server->onNotFound([](AsyncWebServerRequest *request){
    request->send(404);
  });
}
