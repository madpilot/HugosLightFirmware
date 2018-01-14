#include "ConfigEncoder.h"
#include <Arduino.h>

unsigned int chr2hex(unsigned char hex) {
  switch(hex) {
    case '0': return 0x0;
    case '1': return 0x1;
    case '2': return 0x2;
    case '3': return 0x3;
    case '4': return 0x4;
    case '5': return 0x5;
    case '6': return 0x6;
    case '7': return 0x7;
    case '8': return 0x8;
    case '9': return 0x9;
    case 'a': return 0xa;
    case 'b': return 0xb;
    case 'c': return 0xc;
    case 'd': return 0xd;
    case 'e': return 0xe;
    case 'f': return 0xf;
  }
}

unsigned char hex2chr(unsigned int hex) {
  switch(hex) {
    case 0x0: return (unsigned char)'0';
    case 0x1: return (unsigned char)'1';
    case 0x2: return (unsigned char)'2';
    case 0x3: return (unsigned char)'3';
    case 0x4: return (unsigned char)'4';
    case 0x5: return (unsigned char)'5';
    case 0x6: return (unsigned char)'6';
    case 0x7: return (unsigned char)'7';
    case 0x8: return (unsigned char)'8';
    case 0x9: return (unsigned char)'9';
    case 0xa: return (unsigned char)'a';
    case 0xb: return (unsigned char)'b';
    case 0xc: return (unsigned char)'c';
    case 0xd: return (unsigned char)'d';
    case 0xe: return (unsigned char)'e';
    case 0xf: return (unsigned char)'f';
  }
}

config_encode_result ConfigEncoder::encode(unsigned char *in, char **out, int inlen) {
  int outlen = inlen * 2;
  *out = (char *)calloc(outlen + 1, sizeof(char));

  int ptr = 0;
  while(ptr < outlen) {
    (*out)[ptr * 2] = hex2chr(in[ptr] >> 4 & 0xF);
    (*out)[(ptr * 2) + 1] = hex2chr(in[ptr] & 0xF);
  }
  
}

config_encode_result ConfigEncoder::decode(const char *in, unsigned char **out, int inlen) {
  if(inlen % 2 != 0) {
    return CONFIG_ENCODE_ENCODED_INVALID;
  }

  int outlen = inlen / 2;
  *out = (unsigned char *)malloc(outlen * sizeof(unsigned char));
  
  int ptr = 0;
  while(ptr < outlen) {
    (*out)[ptr] = (chr2hex(in[ptr * 2]) << 4) + chr2hex(in[(ptr * 2) + 1]);
    ptr++;
  }

  return CONFIG_ENCODE_OK;
}

