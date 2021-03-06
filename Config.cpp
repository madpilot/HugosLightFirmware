#include "Config.h"

#define CONFIG_FILE_PATH "/config.dat"

Config::Config() {
  this->ssid = NULL;
  this->passkey = NULL;
  this->encryption = 7;
  this->deviceName = NULL;

  this->dhcp = true;
  this->staticIP = NULL;
  this->staticDNS = NULL;
  this->staticGateway = NULL;
  this->staticSubnet = NULL;

	this->set_deviceName("");
	this->set_ssid("");
	this->set_passkey("");
  this->set_hostname("");
  this->set_staticIP("");
  this->set_staticDNS("");
  this->set_staticGateway("");
  this->set_staticSubnet("");
}

char* Config::get_deviceName() {
  return deviceName;
}

char* Config::get_ssid() {
  return ssid;
}

char* Config::get_passkey() {
  return passkey;
}

char* Config::get_hostname() {
  return hostname;
}

int Config::get_encryption() {
  return encryption;
}

bool Config::get_dhcp() {
  return dhcp;
}

char* Config::get_staticIP() {
  return staticIP;
}

char* Config::get_staticDNS() {
  return staticDNS;
}

char* Config::get_staticGateway() {
  return staticGateway;
}

char* Config::get_staticSubnet() {
  return staticSubnet;
}

// Setters
void Config::set_ssid(const char* val) {
  allocString(&this->ssid, val);
}

void Config::set_passkey(const char* val) {
  allocString(&this->passkey, val);
}

void Config::set_encryption(int val) {
  this->encryption = val;
}

void Config::set_deviceName(const char* val) {
  allocString(&this->deviceName, val);
}

void Config::set_hostname(const char* val) {
  allocString(&this->hostname, val);
}

void Config::set_dhcp(bool val) {
  this->dhcp = val;
}

void Config::set_staticIP(const char* val) {
  allocString(&this->staticIP, val);
}

void Config::set_staticDNS(const char* val) {
  allocString(&this->staticDNS, val);
}

void Config::set_staticGateway(const char* val) {
  allocString(&this->staticGateway, val);
}

void Config::set_staticSubnet(const char* val) {
  allocString(&this->staticSubnet, val);
}

bool Config::allocString(char **dest, const char *val) {
  if((*dest) != NULL) {
    free((*dest));
  }

  int len = strlen(val);
  // Strings can't be longer than 255 characters. If they are, truncate them
  if(len > 255) {
    len = 255;
  }

  (*dest) = (char*)malloc(sizeof(char) * (len + 1));
  if((*dest) == NULL) {
    return false;
  }
  strncpy((*dest), val, len);
  return true;
}

int Config::estimateSerializeBufferLength() {
  int size = 2;
  size += strlen(deviceName) + 1;
  size += strlen(ssid) + 1;
  size += strlen(passkey) + 1;
  size += strlen(hostname) + 1;
  size += strlen(staticIP) + 1;
  size += strlen(staticDNS) + 1;
  size += strlen(staticGateway) + 1;
  size += strlen(staticSubnet) + 1;
  return size;
}

void Config::serializeString(unsigned char *buffer, char *string, int *offset) {
  if(string == NULL) {
    buffer[(*offset)++] = 0;
    return;
  }

  int len = strlen(string);
  buffer[(*offset)++] = len;
  memcpy(buffer + (*offset), string, len);
  (*offset) += len;
}

config_result Config::deserializeString(unsigned char *buffer, int bufferlen, char **string, int *offset) {
  int len = buffer[(*offset)++];

  if((*offset) + len > bufferlen) {
    return E_CONFIG_PARSE_ERROR;
  }

  if(*string != NULL) {
    free(*string);
  }

  *string = (char *)malloc(sizeof(char) * (len + 1));

  if(*string == NULL) {
    return E_CONFIG_OUT_OF_MEMORY;
  }

  memcpy(*string, buffer + (*offset), len);
  (*string)[len] = 0;

  (*offset) += len;

  return E_CONFIG_OK;
}

int Config::serialize(unsigned char *buffer) {
  buffer[0] = 0; // Config version number

  // Reserve a byte for booleans and flags
  // bit 0: Encryption
  // bit 1: Encryption
  // bit 2: Encryption
  // bit 3: dhcp
  buffer[1] = 0;
  buffer[1] = buffer[1] | (encryption & 0x07);
  buffer[1] = buffer[1] | (dhcp & 0x01) << 3;

  int offset = 2;
  serializeString(buffer, deviceName, &offset);
  serializeString(buffer, ssid, &offset);
  serializeString(buffer, passkey, &offset);
  serializeString(buffer, hostname, &offset);
  serializeString(buffer, staticIP, &offset);
  serializeString(buffer, staticDNS, &offset);
  serializeString(buffer, staticGateway, &offset);
  serializeString(buffer, staticSubnet, &offset);

  return offset;
}

config_result Config::deserialize(unsigned char *buffer, int length) {
  if(buffer[0] != 0) {
    return E_CONFIG_UNKNOWN_VERSION;
  }

  if(length < 17) {
    return E_CONFIG_CORRUPT;
  }

  encryption = buffer[1] & 0x07;
  dhcp = (buffer[1] >> 3) & 0x01;

  int offset = 2;

  config_result result;

  result = deserializeString(buffer, length, &deviceName, &offset);
  if(result != E_CONFIG_OK) return result;
  result = deserializeString(buffer, length, &ssid, &offset);
  if(result != E_CONFIG_OK) return result;
  result = deserializeString(buffer, length, &passkey, &offset);
  if(result != E_CONFIG_OK) return result;
  result = deserializeString(buffer, length, &hostname, &offset);
  if(result != E_CONFIG_OK) return result;
  result = deserializeString(buffer, length, &staticIP, &offset);
  if(result != E_CONFIG_OK) return result;
  result = deserializeString(buffer, length, &staticDNS, &offset);
  if(result != E_CONFIG_OK) return result;
  result = deserializeString(buffer, length, &staticGateway, &offset);
  if(result != E_CONFIG_OK) return result;
  result = deserializeString(buffer, length, &staticSubnet, &offset);
  if(result != E_CONFIG_OK) return result;

  return E_CONFIG_OK;
}



config_result Config::read() {
  if (SPIFFS.begin()) {
    if (SPIFFS.exists(CONFIG_FILE_PATH)) {
      File configFile = SPIFFS.open(CONFIG_FILE_PATH, "r");

      if (configFile) {
        int length = configFile.size();

        unsigned char *buffer = (unsigned char *)malloc(sizeof(unsigned char) * length);
        if(buffer == NULL) {
          return E_CONFIG_OUT_OF_MEMORY;
        }

        configFile.read(buffer, length);
        deserialize(buffer, length);

        free(buffer);

        configFile.close();
        return E_CONFIG_OK;
      } else {
        configFile.close();
        return E_CONFIG_FILE_OPEN;
      }
    } else {
      return E_CONFIG_FILE_NOT_FOUND;
    }
  } else {
    return E_CONFIG_FS_ACCESS;
  }
}

config_result Config::write() {
  int bufferLength = estimateSerializeBufferLength();
  unsigned char *buffer = (unsigned char *)malloc(sizeof(unsigned char) * bufferLength);
  if(buffer == NULL) {
    return E_CONFIG_OUT_OF_MEMORY;
  }

  int length = serialize(buffer);

  if (SPIFFS.begin()) {
    File configFile = SPIFFS.open(CONFIG_FILE_PATH, "w+");

    if(configFile) {
      configFile.write(buffer, length);
      configFile.close();

      free(buffer);
      return E_CONFIG_OK;
    } else {
      free(buffer);
      return E_CONFIG_FILE_OPEN;
    }
  }
  free(buffer);
  return E_CONFIG_FS_ACCESS;
}
