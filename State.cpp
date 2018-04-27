#include "State.h"
#define STATE_FILE_PATH "/state.dat"

State::State(CRGB *leds, int numLEDS) {
  _numLEDS = numLEDS;
  _lastTick = 0;
  _rawPayload = (uint8_t *)malloc(sizeof(uint8_t) * _numLEDS * 3);
  _leds = leds;

  if(read() != E_STATE_OK) {
    _state = CHSV(0, 0, 0);
    _brightnessState = 255;
    setBrightness(_brightnessState, 0);
    _animationType = 0;
    _animationIndex = 0;

    write();
  }
}

State::~State() {
  free(_rawPayload);
}

uint16_t State::endFrame(long duration) {
  uint16_t end = duration * FPS / 1000;
  if(end < TICKS) {
    end = 1;
  }
  return end;
}

void State::_fade(uint8_t hue, uint8_t saturation, uint8_t value, long duration) {
 CHSV nextState = CHSV(hue, saturation, value);

  _animations.clear();
  for(int i = 0; i < _numLEDS; i++ ) {
    std::vector<Frame<CRGB>> frames;

    CRGB toRGB;
    hsv2rgb_rainbow(nextState, toRGB);

    frames.push_back(Frame<CRGB>(0, CRGB(_leds[i].red, _leds[i].green, _leds[i].blue)));
    frames.push_back(Frame<CRGB>(endFrame(duration), toRGB));
    _animations.push_back(Animation<CRGB>(frames, 2, 0));
  }

  _state = nextState;
  write();
}

bool State::requestAnimationFrame() {
  long now = millis();

  if(now - _lastTick > TICKS) {
    bool changed = false;
    Frame<uint8_t> b;
    _brightnessAnimation.nextFrame(&b);

    if(FastLED.getBrightness() != b.tweenable) {
      changed = true;
      FastLED.setBrightness(b.tweenable);
    }

    if(_animationType == ANIMATION_RAW) {
      for(int i = 0; i < _numLEDS; i++) {
        if(_leds[i].red != *(_rawPayload + (i * 3)) || _leds[i].green != *(_rawPayload + (i * 3) + 1) || _leds[i].blue != *(_rawPayload + (i * 3) + 2)) {
          changed = true;
          _leds[i].red = *(_rawPayload + (i * 3));
          _leds[i].green = *(_rawPayload + (i * 3) + 1);
          _leds[i].blue = *(_rawPayload + (i * 3) + 2);
        }
      }
    } else {
      for(int i = 0; i < _animations.size(); i++) {
        Frame<CRGB> f;
        _animations[i].nextFrame(&f);

        // Don't animate anything if there are no changes
        if(_leds[i].red != f.tweenable.red || _leds[i].green != f.tweenable.green || _leds[i].blue != f.tweenable.blue) {
          changed = true;
          _leds[i].red = f.tweenable.red;
          _leds[i].green = f.tweenable.green;
          _leds[i].blue = f.tweenable.blue;
        }
      }
    }
    return changed;
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
  std::vector<Frame<uint8_t>> frames;

  frames.push_back(Frame<uint8_t>(0, FastLED.getBrightness()));
  frames.push_back(Frame<uint8_t>(endFrame(duration), brightness));

  _brightnessAnimation = Animation<uint8_t>(frames, 2, 0);
  _brightnessState = brightness;
  write();
}

void State::setHSV(uint8_t hue, uint8_t saturation, uint8_t value, long duration) {
  _fade(hue, saturation, value, duration);
}

void State::setRGB(uint8_t red, uint8_t green, uint8_t blue, long duration) {
  CHSV hsv = rgb2hsv_approximate(CRGB(red, green, blue));
  setHSV(hsv.hue, hsv.saturation, hsv.value, duration);
}

void State::setHue(uint8_t hue, long duration) {
  _fade(hue, _state.saturation, _state.value, duration);
}

void State::setSaturation(uint8_t saturation, long duration) {
  _fade(_state.hue, saturation, _state.value, duration);
}

void State::setValue(uint8_t value, long duration) {
  _fade(_state.hue, _state.saturation, value, duration);
}

void State::setAnimation(unsigned int animation) {
  switch(animation) {
  case 0x00:
  case 0x01:
  case 0x02:
    _animationType = animation;
    write();
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

CHSV State::getHSV() {
  return _state;
}

uint8_t State::getBrightness() {
  return _brightnessState;
}

uint8_t State::getAnimationType() {
  return _animationType;
}

uint8_t State::getAnimationIndex() {
  return _animationIndex;
}

void State::serialize(serialized_state *serialized) {
  serialized->hue = _state.hue;
  serialized->saturation = _state.saturation;
  serialized->value = _state.value;
  serialized->brightness = _brightnessState;
  serialized->animationType = _animationType;
  serialized->animationIndex = _animationIndex;
}

int State::deserialize(serialized_state *state) {
  _state.hue = state->hue;
  _state.saturation = state->saturation;
  _state.value = state->value;
  _brightnessState = state->brightness;
  _animationType = state->animationType;
  _animationIndex = state->animationIndex;
}

state_result State::read() {
  return E_STATE_FILE_NOT_FOUND; // Don't write stuff at the moment
  if (SPIFFS.begin()) {
    if (SPIFFS.exists(STATE_FILE_PATH)) {
      File stateFile = SPIFFS.open(STATE_FILE_PATH, "r");

      if (stateFile) {
        int length = stateFile.size();

        uint8_t *buffer = (uint8_t *)malloc(sizeof(uint8_t) * length);
        if(buffer == NULL) {
          return E_STATE_OUT_OF_MEMORY;
        }

        stateFile.read(buffer, length);
        deserialize((serialized_state *)buffer);
        free(buffer);

        stateFile.close();
        return E_STATE_OK;
      } else {
        stateFile.close();
        return E_STATE_FILE_OPEN;
      }
    } else {
      return E_STATE_FILE_NOT_FOUND;
    }
  } else {
    return E_STATE_FS_ACCESS;
  }
}

state_result State::write() {
  return E_STATE_OK; // Don't write stuff at the moment
  int length = 6;
  uint8_t *buffer = (uint8_t *)malloc(sizeof(uint8_t) * length);
  if(buffer == NULL) {
    return E_STATE_OUT_OF_MEMORY;
  }

  serialize((serialized_state *)&buffer);

  if (SPIFFS.begin()) {
    File stateFile = SPIFFS.open(STATE_FILE_PATH, "w+");

    if(stateFile) {
      stateFile.write(buffer, length);
      stateFile.close();

      free(buffer);
      return E_STATE_OK;
    } else {
      free(buffer);
      return E_STATE_FILE_OPEN;
    }
  }
  free(buffer);
  return E_STATE_FS_ACCESS;
}
