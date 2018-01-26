#include "State.h"

State::State(CRGB *leds, int numLEDS) {
  _numLEDS = numLEDS;
  _lastTick = 0;
  _rawPayload = (uint8_t *)malloc(sizeof(uint8_t) * _numLEDS * 3);

  _state = CHSV(0, 0, 0);
  _leds = leds;

  _brightnessState = 255;
  setBrightness(_brightnessState, 0);
}

State::~State() {
  free(_rawPayload);
}

void State::_fade(uint8_t hue, uint8_t saturation, uint8_t value, long duration) {
  uint16_t endFrame = duration * FPS / 1000;
  if(endFrame < TICKS) {
    endFrame = TICKS;
  }
  CHSV nextState = CHSV(hue, saturation, value);

  _animations.clear();
  for(int i = 0; i < _numLEDS; i++ ) {
    std::vector<Frame<CRGB>> frames;

    CRGB toRGB;
    hsv2rgb_rainbow(nextState, toRGB);

    frames.push_back(Frame<CRGB>(0, CRGB(_leds[i].red, _leds[i].green, _leds[i].blue)));
    frames.push_back(Frame<CRGB>(endFrame, toRGB));
    _animations.push_back(Animation<CRGB>(frames, 2, 0));
  }

  _state = nextState;
}

bool State::requestAnimationFrame() {
  long now = millis();

  if(now - _lastTick > TICKS) {
    Frame<uint8_t> b;
    _brightnessAnimation.nextFrame(&b);
    FastLED.setBrightness(b.tweenable);

    if(_animationType == ANIMATION_RAW) {
      for(int i = 0; i < _numLEDS; i++) {
        _leds[i].red = *(_rawPayload + (i * 3));
        _leds[i].green = *(_rawPayload + (i * 3) + 1);
        _leds[i].blue = *(_rawPayload + (i * 3) + 2);
      }
    } else {
      for(int i = 0; i < _animations.size(); i++) {
        Frame<CRGB> f;
        _animations[i].nextFrame(&f);
        _leds[i].red = f.tweenable.red;
        _leds[i].green = f.tweenable.green;
        _leds[i].blue = f.tweenable.blue;
      }
    }
    return true;
  }
  return false;
}

void State::off(int duration) {
  setBrightness(0, duration);
}

void State::on(int duration) {
  setBrightness(255, duration);
}

void State::setBrightness(uint8_t brightness, int duration) {
  uint16_t endFrame = duration * FPS / 1000;
  if(endFrame < TICKS) {
    endFrame = TICKS;
  }

  std::vector<Frame<uint8_t>> frames;

  frames.push_back(Frame<uint8_t>(0, FastLED.getBrightness()));
  frames.push_back(Frame<uint8_t>(endFrame, brightness));

  _brightnessAnimation = Animation<uint8_t>(frames, 2, 0);
  _brightnessState = brightness;
}

void State::setHSV(uint8_t hue, uint8_t saturation, uint8_t value, long duration) {
  _fade(hue, saturation, value, duration);
}

void State::setAnimation(unsigned int animation) {
  switch(animation) {
  case 0x00:
  case 0x01:
  case 0x02:
    _animationType = animation;
    break;
  }
}

void State::raw(uint8_t *payload, int length) {
  for(int i = 0; i < _numLEDS * 3; i++) {
    if(i >= length) {
      _rawPayload[i] = 0x00;
    } else {
      _rawPayload[i] = *(payload + i);
    }
  }
}
