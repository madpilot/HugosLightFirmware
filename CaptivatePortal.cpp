#include "CaptivatePortal.h"
#include <DNSServer.h>
#include <lwip/def.h>

String toStringIp(IPAddress ip) {
  String res = "";
  for (int i = 0; i < 3; i++) {
    res += String((ip >> (8 * i)) & 0xFF) + ".";
  }
  res += String(((ip >> 8 * 3)) & 0xFF);
  return res;
}

CaptivatePortal::CaptivatePortal() {
  udp = new AsyncUDP;
}

CaptivatePortal::~CaptivatePortal() {
  delete udp;
}

captivate_portal_result CaptivatePortal::start(char *name, AsyncWebServer *server) {  
  WiFi.disconnect();
  WiFi.mode(WIFI_AP_STA);
  boolean result = WiFi.softAP(name);
  delay(500);

  setupListener();

  server->onNotFound([](AsyncWebServerRequest *request) {
    String localIP = toStringIp(request->client()->localIP());

    if(String(request->host()) != localIP) {
      request->redirect(String("http://") + localIP + String("/config"));
    } else {
      request->send(404);
    }
  });

  if(result) {
    return E_CAPTIVATE_PORTAL_OK;
  } else {
    return E_CAPTIVATE_PORTAL_FAIL;
  }
}
void CaptivatePortal::stop() {
  
}

void CaptivatePortal::setupListener() {
  Serial.println("Starting DNS Server...");
  udp->listen(53);

  udp->onPacket([](AsyncUDPPacket packet) {
    if(packet.length() > 0) {
      int len = packet.length();
      uint8_t *buffer = (uint8_t *)malloc(sizeof(uint8_t) * len);
      memcpy(buffer, packet.data(), sizeof(uint8_t) * len);
      
      DNSHeader* dnsHeader = (DNSHeader *)buffer;
      
      bool onlyOneQuestion = ntohs(dnsHeader->QDCount) == 1 && dnsHeader->ANCount == 0 && dnsHeader->NSCount == 0 && dnsHeader->ARCount == 0;
      
      if (dnsHeader->QR == DNS_QR_QUERY && dnsHeader->OPCode == DNS_OPCODE_QUERY && onlyOneQuestion) {
        Serial.printf("Received %i\n", len);
        int responseLen = len + 16;
        Serial.println("Is query");

        AsyncUDPMessage *message = new AsyncUDPMessage(responseLen);
        dnsHeader->QR = DNS_QR_RESPONSE;
        dnsHeader->ANCount = dnsHeader->QDCount;
        dnsHeader->QDCount = dnsHeader->QDCount;
        
        message->write(buffer, len);
        message->write((uint8_t)192); //  answer name is a pointer
        message->write((uint8_t)12);  // pointer to offset at 0x00c
  
        message->write((uint8_t)0);   // 0x0001  answer is type A query (host address)
        message->write((uint8_t)1);
      
        message->write((uint8_t)0);   //0x0001 answer is class IN (internet address)
        message->write((uint8_t)1);
       
        message->write((uint8_t)0); // Set TTL to 0x00000001
        message->write((uint8_t)0);
        message->write((uint8_t)0);
        message->write((uint8_t)1);
      
        // Length of RData is 4 bytes (because, in this case, RData is IPv4)
        message->write((uint8_t)0);
        message->write((uint8_t)4);
        
        message->write((uint8_t)packet.localIP()[0]);
        message->write((uint8_t)packet.localIP()[1]);
        message->write((uint8_t)packet.localIP()[2]);
        message->write((uint8_t)packet.localIP()[3]);

        Serial.println("About to send packet");

        size_t rl = packet.send(*message);

        Serial.printf("Sent %i\n", rl);
        delete message;
      }
      
      free(buffer);
    }
  });
}
