#include "Api.h"
#define MAGIC_NUMBER 0x48

int Api::duration(uint8_t *recvBuffer, uint8_t length, uint8_t offset) {
  uint8_t *start = recvBuffer;

  if(length == offset + 2) {
    start += offset;
    return (*(start++) << 8) + *start;
  } else {
    return 0;
  }
}

int Api::dispatch(State *state, uint8_t *payload, int length) {
  uint8_t *recvBuffer = payload;
  uint8_t recvLen = length;

  if(recvLen-- >= 2 && *(recvBuffer++) == (uint8_t)MAGIC_NUMBER) {
    uint8_t d = 0;
    uint8_t command = *(recvBuffer++);
    recvLen--;

    switch(command) {
      case COMMAND_OFF:
        d = duration(recvBuffer, recvLen, 0);
        state->off(d);
        break;
      case COMMAND_ON:
        d = duration(recvBuffer, recvLen, 0);
        state->on(d);
        break;
      case COMMAND_SET_BRIGHTNESS:
        d = duration(recvBuffer, recvLen, 1);
        if(recvLen < 1) return 0;
        state->setBrightness(*recvBuffer, d);
        break;
      case COMMAND_SET_HUE:
        d = duration(recvBuffer, recvLen, 1);
        if(recvLen < 1) return 0;
        state->setHue(*recvBuffer, d);
        break;
      case COMMAND_SET_SATURATION:
        d = duration(recvBuffer, recvLen, 1);
        if(recvLen < 1) return 0;
        state->setSaturation(*recvBuffer, d);
        break;
      case COMMAND_SET_VALUE:
        d = duration(recvBuffer, recvLen, 1);
        if(recvLen < 1) return 0;
        state->setValue(*recvBuffer, d);
        break;
      case COMMAND_SET_HSV:
        d = duration(recvBuffer, recvLen, 3);
        if(recvLen < 3) return 0;
        state->setHSV(*(recvBuffer++), *(recvBuffer++), *recvBuffer, d);
        break;
      case COMMAND_SET_RGB:
        d = duration(recvBuffer, recvLen, 3);
        if(recvLen < 3) return 0;
        state->setRGB(*(recvBuffer++), *(recvBuffer++), *recvBuffer, d);
        break;
      case COMMAND_SET_ANIMATION:
        if(recvLen < 1) return 0;
        state->setAnimation((unsigned int)*recvBuffer);
        break;
      case COMMAND_RAW:
        // Command Raw doesn't send the current state as a response.
        state->raw(recvBuffer, recvLen);
        return 0;
    }

    uint8_t *sendBuffer = payload;
    for(int i = 0; i < length; i++) {
      *(payload + i) = 0x00;
    }

    int sendLen = 8 * sizeof(uint8_t);
    *(sendBuffer++) = (uint8_t)MAGIC_NUMBER;
    *(sendBuffer++) = RESPONSE;

    state->serialize((serialized_state *)(sendBuffer));
    return sendLen;
  }
  return 0;
}
