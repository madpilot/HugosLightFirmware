#include <FastLED.h>

#include <WiFiUdp.h>
#include <ArduinoOTA.h>
#include <ESPAsyncWebServer.h>

#include "Config.h"
#include "ConfigServer.h"
#include "WifiManager.h"
#include "CaptivatePortal.h"

#define CONFIG_AP_SSID "hugos-lights"

#define DATA_PIN    15
#define BUTTON 13

#define LED_TYPE    WS2811
#define COLOR_ORDER GRB
#define NUM_LEDS    14

CRGB leds[NUM_LEDS];

Config config;
AsyncWebServer server(80);

bool configMode = false;
bool otaMode = false;

WifiManager wifiManager(&config);

void buttonPress() {

}

config_result configSetup() {
  config_result result = config.read();
  switch(result) {
    case E_CONFIG_OK:
      Serial.println("Config read");
      break;
    case E_CONFIG_FS_ACCESS:
      Serial.println("E_CONFIG_FS_ACCESS: Couldn't access file system");
      break;
    case E_CONFIG_FILE_NOT_FOUND:
      Serial.println("E_CONFIG_FILE_NOT_FOUND: File not found");
      break;
    case E_CONFIG_FILE_OPEN:
      Serial.println("E_CONFIG_FILE_OPEN: Couldn't open file");
      break;
    case E_CONFIG_PARSE_ERROR:
      Serial.println("E_CONFIG_PARSE_ERROR: File was not parsable");
      break;
  }
  return result;
}

void wifiSetup() {
  while(wifiManager.loop() != E_WIFI_OK) {
    Serial.println("Could not connect to WiFi. Will try again in 5 seconds");
    delay(5000);
  }
  Serial.println("Connected to WiFi");
}

CaptivatePortal *portal;
void captivatePortalSetup() {
  portal = new CaptivatePortal();
  if(portal->start(CONFIG_AP_SSID, &server) == E_CAPTIVATE_PORTAL_OK) {
    Serial.printf("Captivate Portal set up. Name: %s\n", CONFIG_AP_SSID);
  } else {
    Serial.println("Unable to setup Captivate Portal");
  }
}

void setup() {
  Serial.begin(115200);
  FastLED.addLeds<LED_TYPE,DATA_PIN,COLOR_ORDER>(leds, NUM_LEDS).setCorrection(TypicalLEDStrip);

  for(int i = 0; i < NUM_LEDS; i++) {
    leds[i] = CRGB::Black;
  }
  FastLED.show();
  
  pinMode(BUTTON, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(BUTTON), buttonPress, FALLING);
 
  config_result configResult = configSetup();
  ConfigServer::setup(&server);
  if(configResult != E_CONFIG_OK) {
    configMode = true;
    captivatePortalSetup();
    server.begin();
    return;
  }
  server.begin();

  wifiSetup();
  
  /*
  if(digitalRead(BUTTON) == 0) {
    otaMode = true;
    ArduinoOTA.setPort(8266);
    ArduinoOTA.setHostname("hugos-light");
    ArduinoOTA.onStart([]() {
      Syslogger.send(SYSLOG_DEBUG, "Starting OTA Update.");
    });
    ArduinoOTA.onEnd([]() {
      Syslogger.send(SYSLOG_DEBUG, "OTA Update complete.");
    });
    ArduinoOTA.onProgress([](unsigned int progress, unsigned int total) {
      unsigned int percent = (progress / (total / 100));

      if(percent % 2 == 0) {
        digitalWrite(LED, LOW);
      }
      if(percent % 2 == 1) {
        digitalWrite(LED, HIGH);
      }

      if(percent % 10 == 0) {
        char *str = "OTA Update progress: 100%";
        sprintf(str, "OTA Update progress: %u%%", percent);
        Syslogger.send(SYSLOG_DEBUG, str);
      }
    });
    ArduinoOTA.onError([](ota_error_t error) {
      switch(error) {
        case OTA_AUTH_ERROR:
          Syslogger.send(SYSLOG_DEBUG, "OTA Update failed: Authentication failed.");
          break;
        case OTA_BEGIN_ERROR:
          Syslogger.send(SYSLOG_DEBUG, "OTA Update failed: Begin failed.");
          break;
        case OTA_CONNECT_ERROR:
          Syslogger.send(SYSLOG_DEBUG, "OTA Update failed: Connect failed.");
          break;
        case OTA_RECEIVE_ERROR:
          Syslogger.send(SYSLOG_DEBUG, "OTA Update failed: Receive failed.");
          break;
        case OTA_END_ERROR:
          Syslogger.send(SYSLOG_DEBUG, "OTA Update failed: End failed.");
          break;
      }
    });
    ArduinoOTA.begin();
    
    Syslogger.send(SYSLOG_DEBUG, "Ready for OTA update. Push to coffee-machine.local:8266");
    return;
  }
  */  
}

void loop() {
  // put your main code here, to run repeatedly:
   if(otaMode) {
    ArduinoOTA.handle();
    return;
  }

  if(configMode) {
    
    return;
  }

  /*
  if(buttonPressed) {
    buttonPressed = false;
  }
  */

  int wifiRes = wifiManager.loop();
  if(wifiRes != E_WIFI_OK) {
    //Serial.print("Unable to connect to WiFI: ");
    //Serial.println(wifiRes);
  }
}
