#include "Animation.h"
#include <stdio.h>

Animation::Animation(animation animation) {
  _frames = animation.frames;
  _number = animation.num_frames;
}

Animation::~Animation() {}

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

void Animation::frame(int num, struct frame* result) {
  int low, high, middle;
  low = 0;
  high = _number - 1;

  struct frame *needle = NULL;

  while(true) {
    middle = (low + high) / 2;
    needle = _frames + middle;

    if(needle->number == num) {
      // Frame found
      result->led.red = needle->led.red;
      result->led.green = needle->led.green;
      result->led.blue = needle->led.blue;
      return;

    } else if(low > high) {
      struct frame* prev = NULL;
      struct frame* next = NULL;

      // Couldn't find the frame, so we need to find the prev/next
      if(needle->number > num) {
        prev = needle - 1;
        next = needle;
      } else {
        prev = needle;
        next = needle + 1;
      }

      result->number = num;

      float increment = _calculateIncrement(num, prev->number, next->number);

      result->led.red = _tween(prev->led.red, next->led.red, increment);
      result->led.green = _tween(prev->led.green, next->led.green, increment);
      result->led.blue = _tween(prev->led.blue, next->led.blue, increment);
      return;

    } else if(needle->number > num) {
      high = middle - 1;
    } else if(needle->number < num) {
      low = middle + 1;
    }
  }
}
