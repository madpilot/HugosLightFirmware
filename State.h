#ifndef STATE_h
#define STATE_h
#include "Animation.h"
#include "Frame.h"
#include <FastLED.h>
#include <vector>

#define FPS 100
#define TICKS 1000 / FPS

#define COMMAND_OFF 0x00
#define COMMAND_ON 0x01
#define COMMAND_GET_STATE 0x02
#define COMMAND_SET_BRIGHTNESS 0x03
#define COMMAND_SET_HUE 0x04
#define COMMAND_SET_SATURATION 0x05
#define COMMAND_SET_VALUE 0x06
#define COMMAND_SET_HSV 0x07
#define COMMAND_SET_RGB 0x08
#define COMMAND_SET_ANIMATION 0x09
#define COMMAND_LIST_ANIMATIONS 0x0A
#define COMMAND_RAW 0x0B

#define ANIMATION_NONE   0x00
#define ANIMATION_RAW    0x01
#define ANIMATION_CUSTOM 0x02

class State {
  public:
    State(CRGB *leds, int numLEDS);
    ~State();

    void off(int duration);
    void on(int duration);
    void setBrightness(uint8_t brightness, int duration);

    void setHSV(uint8_t hue, uint8_t saturation, uint8_t value, long duration);

    void setAnimation(unsigned int animation);
    void raw(uint8_t *payload, int length);

    bool requestAnimationFrame();
  private:
    int _numLEDS;
    long _lastTick;

    CHSV _state;
    CRGB *_leds;

    std::vector<Animation> _animations;

    int _animationType;
    int _animationIndex;

    uint8_t *_rawPayload;

    void _fade(uint8_t hue, uint8_t saturation, uint8_t value, long duration);
};
#endif