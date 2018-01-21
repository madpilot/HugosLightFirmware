#include "Animation.h"
#include <stdio.h>

Animation::Animation(std::vector<Frame> frames, uint8_t numKeyFrames, unsigned int repeat) {
  _frames = frames;
  _times = repeat + 1;
  _numberKeyFrames = numKeyFrames;
  _numberFrames = _frames[_frames.size() - 1].number;
  _currentFrame = 0;
  _loopForever = false;
}

Animation::Animation(std::vector<Frame> frames, uint8_t numKeyFrames) {
  _frames = frames;
  _times = 0;
  _currentFrame = 0;
  _numberKeyFrames = numKeyFrames;
  _loopForever = true;
}

float Animation::_calculateIncrement(int num, int prev, int next) {
  float increment = 0;

  if(next != prev) {
    increment = (float)(num - prev) / (float)(next - prev);
  }

  return increment;
}

int Animation::_tween(int prev, int next, float increment) {
  return prev + (int)((float)next - prev) * increment;
}

void Animation::frame(int num, Frame* result) {
  int low, high, middle;
  low = 0;
  high = _numberKeyFrames - 1;

  Frame needle;

  while(true) {
    middle = (low + high) / 2;
    needle = _frames[middle];

    if(needle.number == num) {
      // Frame found
      result->led.red = needle.led.red;
      result->led.green = needle.led.green;
      result->led.blue = needle.led.blue;
      return;

    } else if(low > high) {
      Frame prev;
      Frame next;

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

      result->led.red = _tween(prev.led.red, next.led.red, increment);
      result->led.green = _tween(prev.led.green, next.led.green, increment);
      result->led.blue = _tween(prev.led.blue, next.led.blue, increment);
      return;

    } else if(needle.number > num) {
      high = middle - 1;
    } else if(needle.number < num) {
      low = middle + 1;
    }
  }
}

unsigned int Animation::nextFrame(Frame *result) {
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
