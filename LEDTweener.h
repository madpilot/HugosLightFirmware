#ifndef LED_FRAME_h
#define LED_FRAME_h
#include <FastLED.h>

class LEDTweener {
  public:
    LEDTweener();

    static CRGB tween(CRGB *prev, CRGB *next, float increment) {
      uint8_t red = Tweener::tween(prev->led.red, next->led.red, increment);
      uint8_t green = Tweener::tween(prev->led.green, next->led.green, increment);
      uint8_t blue = Tweener::tween(prev->led.blue, next->led.blue, increment);

      return CRGB(red, green, blue);
    }
};
#endif
