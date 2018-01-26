#define FASTLED_ALLOW_INTERRUPTS 0

#include <FastLED.h>

#include <WiFiUdp.h>
#include <ArduinoOTA.h>
#include <ESPAsyncWebServer.h>
#include <ESP8266mDNS.h>
#include <Ticker.h>

#include "Config.h"
#include "ConfigServer.h"
#include "WifiManager.h"
#include "CaptivatePortal.h"
#include "State.h"

#define CONFIG_AP_SSID "hugos-lights"

#define DATA_PIN    15
#define BUTTON      5

#define LED_TYPE    WS2811
#define COLOR_ORDER GRB
#define NUM_LEDS    14

CRGB leds[NUM_LEDS];
State state(leds, NUM_LEDS);

Config config;
AsyncWebServer webServer(80);

bool configMode = false;

WifiManager wifiManager(&config);

Ticker doubleClickTicker;
Ticker holdClickTicker;

volatile bool isButtonDown = false;
volatile int buttonPresses = 0;

#define CLICK_SINGLE 0x01
#define CLICK_DOUBLE 0x02
#define CLICK_HOLD 0x03

#define UDP_PORT 7269
#define MAGIC_NUMBER 0x48

uint8_t animation = ANIMATION_RAW;
WiFiUDP Udp;

void buttonEvent(int type) {
  if(type == CLICK_HOLD) {
    if(isButtonDown && buttonPresses == 0) {
      Serial.println("Click: Hold");
    }

    // Regardless, reset the counters...
    isButtonDown = false;
    buttonPresses = 0;

    doubleClickTicker.detach();
    holdClickTicker.detach();
  } else if(type == CLICK_DOUBLE) {
    if(buttonPresses >= 2) {
      isButtonDown = false;
      buttonPresses = 0;
      doubleClickTicker.detach();
      holdClickTicker.detach();

      Serial.println("Click: Double");
    } else if(buttonPresses == 1) {
      isButtonDown = false;
      buttonPresses = 0;
      doubleClickTicker.detach();
      holdClickTicker.detach();

      Serial.println("Click: Single");
    }
  }
}

void buttonDown() {
  if(buttonPresses == 0) {
    doubleClickTicker.once_ms(500, buttonEvent, CLICK_DOUBLE);
    holdClickTicker.once(2, buttonEvent, CLICK_HOLD);
  }

  isButtonDown = true;
}

void buttonUp() {
  if(isButtonDown) {
    buttonPresses += 1;
    isButtonDown = false;
  }
}

void buttonChange() {
  if(digitalRead(BUTTON) == 1) {
    buttonUp();
  } else {
    buttonDown();
  }
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
  MDNS.begin(config.get_deviceName());
  Serial.printf("Registered as %s.local\n", config.get_deviceName());
  MDNS.addService("http", "tcp", 80);
  // TODO: Add a Service for discovery
}

CaptivatePortal *portal;
void captivatePortalSetup() {
  portal = new CaptivatePortal();
  if(portal->start(CONFIG_AP_SSID, &webServer) == E_CAPTIVATE_PORTAL_OK) {
    Serial.printf("Captivate Portal set up. Name: %s\n", CONFIG_AP_SSID);
  } else {
    Serial.println("Unable to setup Captivate Portal");
  }
}

void setup() {
  Serial.begin(115200);
  FastLED.addLeds<LED_TYPE,DATA_PIN,COLOR_ORDER>(leds, NUM_LEDS).setCorrection(TypicalLEDStrip);

  // Turn off all the LEDs for boot
  for(int i = 0; i < NUM_LEDS; i++) {
    leds[i] = CRGB::Black;
  }
  FastLED.show();

  pinMode(BUTTON, INPUT);
  attachInterrupt(digitalPinToInterrupt(BUTTON), buttonChange, CHANGE);

  config_result configResult = configSetup();
  ConfigServer::setup(&webServer, &config);

  // Start in captivate mode if there was a problem reading the config,
  // or the hardware button is held down on boot
  if(configResult != E_CONFIG_OK || digitalRead(BUTTON) == 0) {
    configMode = true;
    captivatePortalSetup();
    webServer.begin();
    return;
  }

  wifiSetup();

  webServer.begin();
  Udp.begin(UDP_PORT);
}

void loop() {
  if(configMode) {
    return;
  }

  int length = Udp.parsePacket();
  if(length) {
    uint8_t *payload = (uint8_t *)malloc(sizeof(uint8_t) * length);
    Udp.read(payload, length);

    if(length >= 2 && *(payload++) == (uint8_t)MAGIC_NUMBER) {
      uint16_t duration = 0;
      int strippedLength = length - 2;

      switch(*(payload++)) {
        case COMMAND_OFF:
          if(strippedLength >= 2) {
            duration = (*(payload) << 8) + *(payload + 1);
          }
          state.off(duration);
          break;
        case COMMAND_ON:
          if(strippedLength >= 2) {
            duration = (*(payload) << 8) + *(payload + 1);
          }
          state.on(duration);
          break;
        case COMMAND_SET_BRIGHTNESS:
          if(strippedLength >= 3) {
            duration = (*(payload + 1) << 8) + *(payload + 2);
          }
          if(strippedLength >= 1) {
            state.setBrightness(*payload, duration);
          }
          break;
        case COMMAND_SET_HSV:
          if(strippedLength >= 5) {
            duration = (*(payload + 3) << 8) + *(payload + 4);
          }
          state.setHSV(*payload, *(payload + 1), *(payload + 2), duration);
          break;
        case COMMAND_SET_ANIMATION:
          if(strippedLength > 0) {
            state.setAnimation((unsigned int)*payload);
          }
          break;
        case COMMAND_RAW:
          state.raw(payload, strippedLength);
          break;
      }
    }
    free(payload);
  }

  if(state.requestAnimationFrame()) {
    // When the next animation frame is ready, render the LEDS;
    FastLED.show();
  }


}
