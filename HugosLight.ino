#define FASTLED_ALLOW_INTERRUPTS 0
#define FASTLED_INTERRUPT_RETRY_COUNT 3
#define INTERRUPT_THRESHOLD 1
#define FASTLED_INTERNAL

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
#include "Api.h"

#define DATA_PIN    15
#define BUTTON      5

#define LED_TYPE    WS2811
#define COLOR_ORDER GRB
#define NUM_LEDS    6

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
  // Turn off WIFI sleep to remove some jitter - apparently by default the ESP8266 uses a low power
  // mode where it goes to sleep between beacons, causing an elongated interupt, which messes with
  // the LED timing.
  WiFi.setSleepMode(WIFI_NONE_SLEEP);

  while(wifiManager.loop() != E_WIFI_OK) {
    Serial.println("Could not connect to WiFi. Will try again in 5 seconds");
    delay(5000);
  }

  Serial.println("Connected to WiFi");
  MDNS.begin(config.get_hostname());

  char *id = "xxxxxx";
  sprintf(id, "%06x", ESP.getChipId());

  Serial.printf("Registered as %s.local\n", config.get_hostname());
  MDNS.addService("http", "tcp", 80);
  MDNS.addService("felux", "udp", UDP_PORT);
  MDNS.addServiceTxt("felux", "udp", "device_name", (const char *)config.get_deviceName());
  MDNS.addServiceTxt("felux", "udp", "leds", (const char *)String(NUM_LEDS).c_str());
  MDNS.addServiceTxt("felux", "udp", "id", (const char *)id);

  Serial.printf("Felux ID: %i", ESP.getChipId());
}

CaptivatePortal *portal;
void captivatePortalSetup() {
  portal = new CaptivatePortal();
  char* ssid = "felux_xxxxxx";
  sprintf(ssid, "felux_%06X", ESP.getChipId());

  if(portal->start(ssid, &webServer) == E_CAPTIVATE_PORTAL_OK) {
    Serial.printf("Captivate Portal set up. Name: %s\n", ssid);
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

    for(int i = 0; i < NUM_LEDS; i++) {
      leds[i] = CRGB::Teal;
    }
    FastLED.show();

    return;
  }

  wifiSetup();
  webServer.begin();
  Udp.begin(UDP_PORT);
}

uint8_t payload[UDP_TX_PACKET_MAX_SIZE];
void loop() {
  if(configMode) {
    return;
  }

  int length = Udp.parsePacket();
  if(length && length <= UDP_TX_PACKET_MAX_SIZE) {
    Udp.read(payload, length);

    // The state is returned on every query (Except Raw mode)
    int len = Api::dispatch(&state, &payload[0], length);
    if(len > 0) {
      Udp.beginPacket(Udp.remoteIP(), Udp.remotePort());
      Udp.write(payload, len);
      Udp.endPacket();

      Udp.beginPacket(IPAddress(255, 255, 255, 255), UDP_PORT);
      Udp.write(payload, len);
      Udp.endPacket();
    }
  }

  if(state.requestAnimationFrame()) {
    // When the next animation frame is ready, render the LEDS;
    FastLED.show();
  }
}
