#ifndef ANIMATION_h
#define ANIMATION_h
#include <FastLED.h>
#include <stdint.h>

struct frame {
  uint16_t number;
  CRGB led;
};

struct animation {
  uint8_t num_frames;
  struct frame *frames;
};

class Animation {
  public:
    Animation(animation animation);
    ~Animation();
    void frame(int num, frame* result);

  private:
    struct frame* _frames;
    int _number;
    float _calculateIncrement(int num, int prev, int next);
    int _tween(int prev, int next, float increment);
};
#endif
