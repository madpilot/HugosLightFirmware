#ifndef Config_h
#define Config_h

#define config_result             uint8_t
#define E_CONFIG_OK               0
#define E_CONFIG_FS_ACCESS        1
#define E_CONFIG_FILE_NOT_FOUND   2
#define E_CONFIG_FILE_OPEN        3
#define E_CONFIG_PARSE_ERROR      4
#define E_CONFIG_OUT_OF_MEMORY    5
#define E_CONFIG_UNKNOWN_VERSION  6
#define E_CONFIG_CORRUPT          7

#include <FS.h>

class Config {
  public:
    Config();

    // Getters
    char* get_deviceName();
    char* get_hostname();
    char* get_ssid();
    char* get_passkey();
    int get_encryption();

    bool get_dhcp();
    char* get_staticIP();
    char* get_staticDNS();
    char* get_staticGateway();
    char* get_staticSubnet();

    // Setters
    void set_deviceName(const char* val);
    void set_ssid(const char* val);
    void set_passkey(const char* val);
    void set_encryption(int val);

    void set_hostname(const char* val);
    void set_dhcp(bool val);;
    void set_staticIP(const char* val);
    void set_staticDNS(const char* val);
    void set_staticGateway(const char* val);
    void set_staticSubnet(const char* val);

    config_result write();
    config_result read();

    int serialize(unsigned char *buffer);
    config_result deserialize(unsigned char *buffer, int length);
    int estimateSerializeBufferLength();

  private:
    char* ssid;
    char* passkey;
    int encryption;
    char* deviceName;

    bool dhcp;
    char *hostname;
    char* staticIP;
    char* staticDNS;
    char* staticGateway;
    char* staticSubnet;

    bool allocString(char **dest, const char *val);
    config_result deserializeString(unsigned char *buffer, int bufferlen, char **string, int *offset);

    void serializeString(unsigned char *buffer, char *string, int *offset);
};



#endif
