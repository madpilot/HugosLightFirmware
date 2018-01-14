#ifndef ConfigEncoder_h
#define ConfigEncoder_h
#include <stdint.h>

#define config_encode_result          uint8_t
#define CONFIG_ENCODE_OK              0x00
#define CONFIG_ENCODE_ENCODED_INVALID 0x01

class ConfigEncoder {
  public:
    static config_encode_result encode(unsigned char *in, char **out, int inlen);
    static config_encode_result decode(const char *in, unsigned char **out, int inlen);
};

#endif
