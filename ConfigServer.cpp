#include "ConfigServer.h"
#include <ESP8266mDNS.h>
#include "config.html.h"

void doUpload(AsyncWebServerRequest *request, const String& filename, size_t index, uint8_t *data, size_t len, bool final) {}

void ConfigServer::setup(AsyncWebServer *server, Config *config) {
  server->on("/config", HTTP_GET, [](AsyncWebServerRequest *request) {
    AsyncWebServerResponse *response = request->beginResponse_P(200, "text/html; charset=utf-8", config_html_gz, config_html_gz_len);
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

  server->on("/config/update", HTTP_POST, [config](AsyncWebServerRequest *request) {
    if(request->hasParam("config.dat", true)) {
      AsyncWebParameter* p = request->getParam("config.dat", true);

      const char *postedConfig = p->value().c_str();
      int postedConfigLen = strlen(postedConfig);
      Serial.printf("Config: %s\n", postedConfig);

      int configBufferLen = postedConfigLen / 2;
      unsigned char *configBuffer;

      if(ConfigEncoder::decode((char *)postedConfig, &configBuffer, postedConfigLen) == CONFIG_ENCODE_OK) {
        for(int i = 0; i < configBufferLen; i++) {
          Serial.printf("%x", configBuffer[i]);
        }
        Serial.println("");
        config_result deserializeResult = config->deserialize(configBuffer, configBufferLen);

        if(deserializeResult == E_CONFIG_OK && config->write() == E_CONFIG_OK) {
          request->send(200);
          return;
        } else {
          Serial.printf("Error: %i\n", deserializeResult);
          request->send(500);
          return;
        }
      } else {
        request->send(500);
        return;
      }
    }
    request->send(200);
  }, doUpload);

  server->on("/config.dat", HTTP_GET, [config](AsyncWebServerRequest *request) {
    if(config->read() == E_CONFIG_OK) {
      int configBufferLen = config->estimateSerializeBufferLength();
      unsigned char *configBuffer = (unsigned char *)malloc(configBufferLen * sizeof(unsigned char));

      if(config->serialize(configBuffer) != E_CONFIG_OK) {
        free(configBuffer);
        request->send(500);
        return;
      }

      char *out;
      if(ConfigEncoder::encode(configBuffer, &out, configBufferLen) == CONFIG_ENCODE_OK) {
        AsyncWebServerResponse *response = request->beginResponse(200, "text/plain", String(out));
        response->addHeader("Cache-Control", "no-cache, no-store, must-revalidate");
        response->addHeader("Pragma", "no-cache");
        response->addHeader("Expires", "-1");
        request->send(response);

        free(out);
      } else {
        request->send(500);
      }

      free(configBuffer);
    } else {
      request->send(404);
    }
  });

  server->onFileUpload([](AsyncWebServerRequest *request, const String& filename, size_t index, uint8_t *data, size_t len, bool final) {
    if(request->url() == "/firmware") {
      if(index == 0) {
        Serial.println("Firmware upload started");
        uint32_t maxSketchSpace = (ESP.getFreeSketchSpace() - 0x1000) & 0xFFFFF000;
        if(!Update.begin(maxSketchSpace)) {
          Serial.println("Firmware upload could not start");
          request->send(500);
        }
      } else if(final) {
        if(Update.end(true)) {
          Serial.println("Firmware upload complete. Restarting.");
          request->send(200);
          delay(100);
          ESP.restart();
        } else {
          Serial.println("Firmware upload failed");
          request->send(500);
        }
      } else {
        if(Update.write(data, len) != len) {
          Serial.println("Firmware upload aborted");
          request->send(500);
        }
      }
    }
  });

  server->onNotFound([](AsyncWebServerRequest *request){
    request->send(404);
  });
}
