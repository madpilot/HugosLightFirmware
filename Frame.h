#ifndef FRAME_h
#define FRAME_h
#include <FastLED.h>

template <class T>
class Frame {
  public:
    Frame() {
      number = 0;
      tweenable = CRGB(0, 0, 0);
    }
    Frame(uint16_t n, T t) {
      number = n;
      tweenable = t;
    }

    uint16_t number;
    T tweenable;
};
#endif
