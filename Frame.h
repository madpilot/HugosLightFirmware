#ifndef FRAME_h
#define FRAME_h
#include <FastLED.h>
class Frame {
  public:
    Frame();
    Frame(uint16_t n, CRGB l);
    uint16_t number;
    CRGB led;
};
#endif
