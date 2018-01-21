#include "Frame.h"

Frame::Frame() {
  number = 0;
  led = CRGB(0, 0, 0);
}

Frame::Frame(uint16_t n, CRGB l) {
  number = n;
  led = l;
}
