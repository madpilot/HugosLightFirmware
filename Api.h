#ifndef API_h
#define API_h
#include "State.h"

class Api {
  public:
    static int dispatch(State *state, uint8_t *payload, int length);
  private:
    static uint16_t duration(uint8_t *recvBuffer, uint8_t length, uint8_t offset);
};
#endif
