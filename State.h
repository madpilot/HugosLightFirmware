#ifndef STATE_h
#define STATE_h
#include "Animation.h"
#include "Frame.h"
#include <FastLED.h>
#include <vector>

#define FPS 100
#define TICKS 1000 / FPS

#define RESPONSE 0xF0

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

typedef struct serialized_state {
  uint8_t hue;
  uint8_t saturation;
  uint8_t value;
  uint8_t brightness;
  uint8_t animationType;
  uint8_t animationIndex;
};

class State {
  public:
    State(CRGB *leds, int numLEDS);
    ~State();

    void off(int duration);
    void on(int duration);
    void setBrightness(uint8_t brightness, int duration);

    void setHSV(uint8_t hue, uint8_t saturation, uint8_t value, long duration);
    void setRGB(uint8_t red, uint8_t green, uint8_t blue, long duration);

    void setHue(uint8_t hue, long duration);
    void setSaturation(uint8_t saturation, long duration);
    void setValue(uint8_t value, long duration);

    void setAnimation(unsigned int animation);
    void raw(uint8_t *payload, int length);

    bool requestAnimationFrame();

    CHSV getHSV();
    uint8_t getBrightness();
    uint8_t getAnimationType();
    uint8_t getAnimationIndex();

    void serialize(serialized_state *serialized);
    int deserialize(serialized_state *state);
  private:
    int _numLEDS;
    long _lastTick;

    CHSV _state;
    CRGB *_leds;
    uint8_t _brightnessState;

    std::vector<Animation<CRGB>> _animations;
    Animation<uint8_t> _brightnessAnimation;

    uint8_t _animationType;
    uint8_t _animationIndex;

    uint8_t *_rawPayload;
    uint16_t endFrame(long duration);

    void _fade(uint8_t hue, uint8_t saturation, uint8_t value, long duration);
};
#endif
