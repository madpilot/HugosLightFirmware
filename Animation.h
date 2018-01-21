#ifndef ANIMATION_h
#define ANIMATION_h
#include <FastLED.h>
#include <stdint.h>
#include "Frame.h"
#include <vector>

struct frame {
  uint16_t number;
  CRGB led;
};

class Animation {
  public:
    Animation(std::vector<Frame> frames, uint8_t numKeyFrames);
    Animation(std::vector<Frame> frames, uint8_t numKeyFrames, unsigned int repeat);
    void frame(int num, Frame* result);
    unsigned int nextFrame(Frame *result);

  private:
    std::vector<Frame> _frames;
    int _numberKeyFrames;
    int _numberFrames;
    float _calculateIncrement(int num, int prev, int next);
    int _tween(int prev, int next, float increment);
    unsigned int _currentFrame;
    int _times;
    bool _loopForever;
};
#endif
