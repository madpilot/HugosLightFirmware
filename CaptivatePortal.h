#ifndef CAPTIVATE_PORTAL_h
#define CAPTIVATE_PORTAL_h

#include <ESP8266WiFi.h>
#include <ESPAsyncWebServer.h>
#include <ESPAsyncUDP.h>

#define captivate_portal_result   uint8_t
#define E_CAPTIVATE_PORTAL_OK     0x00
#define E_CAPTIVATE_PORTAL_FAIL   0x01

class CaptivatePortal {
  public:
    CaptivatePortal();
    ~CaptivatePortal();
    captivate_portal_result start(char *name, AsyncWebServer *server);
    void stop();
  private:
    AsyncUDP *udp;
    void setupListener();
};
#endif
