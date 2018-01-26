#ifndef ANIMATION_h
#define ANIMATION_h
#include <stdint.h>
#include <vector>
#include "Frame.h"

template <class T>
class Animation {
  public:
    Animation() {
      _times = 0;
      _currentFrame = 0;
      _numberKeyFrames = 0;
      _loopForever = false;
    }

    Animation(std::vector<Frame<T>> frames, uint8_t numKeyFrames) {
      _frames = frames;
      _times = 0;
      _currentFrame = 0;
      _numberKeyFrames = numKeyFrames;
      _loopForever = true;
    }


    Animation(std::vector<Frame<T>> frames, uint8_t numKeyFrames, unsigned int repeat) {
      _frames = frames;
      _times = repeat + 1;
      _numberKeyFrames = numKeyFrames;
      _numberFrames = _frames[_frames.size() - 1].number;
      _currentFrame = 0;
      _loopForever = false;
    }

    void frame(int num, Frame<T>* result) {
      int low, high, middle;
      low = 0;
      high = _numberKeyFrames - 1;

      Frame<T> needle;

      while(true) {
        middle = (low + high) / 2;
        needle = _frames[middle];

        if(needle.number == num) {
          // Frame found
          memmove(&(result->tweenable), &(needle.tweenable), sizeof(T));
          return;

        } else if(low > high) {
          Frame<T> prev;
          Frame<T> next;

          // Couldn't find the frame, so we need to find the prev/next
          if(needle.number > num) {
            prev = _frames[middle - 1];
            next = _frames[middle];
          } else {
            prev = _frames[middle];
            next = _frames[middle + 1];
          }

          result->number = num;

          float increment = _calculateIncrement(num, prev.number, next.number);
          result->tweenable = _tween(prev.tweenable, next.tweenable, increment);
          return;

        } else if(needle.number > num) {
          high = middle - 1;
        } else if(needle.number < num) {
          low = middle + 1;
        }
      }

    }

    unsigned int nextFrame(Frame<T> *result) {
      // Even if we aren't looping, we need to hand back a frame
      if(!_loopForever && _times == 0) {
        // Return the LAST frame if the loop has completed
        frame(_numberFrames, result);
        return _numberFrames;
      }

      frame(_currentFrame, result);

      _currentFrame++;

      if(_currentFrame > _numberFrames) {
        _currentFrame = 0;

        if(!_loopForever) {
          _times--;
        }
      }

      return _currentFrame;
    }

  private:
    std::vector<Frame<T>> _frames;
    int _numberKeyFrames;
    int _numberFrames;

    unsigned int _currentFrame;
    int _times;
    bool _loopForever;

    float _calculateIncrement(int num, int prev, int next) {
      float increment = 0;

      if(next != prev) {
        increment = (float)(num - prev) / (float)(next - prev);
      }

      return increment;
    }

    uint8_t _tween(int prev, int next, float increment) {
      return (uint8_t)(prev + (((float)next - (float)prev) * increment));
    }

    CRGB _tween(CRGB prev, CRGB next, float increment) {
      uint8_t r = _tween(prev.red, next.red, increment);
      uint8_t g = _tween(prev.green, next.green, increment);
      uint8_t b = _tween(prev.blue, next.blue, increment);

      return CRGB(r, g, b);
    }

};
#endif
