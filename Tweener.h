#ifndef TWEENER_h
#define TWEENER_h
class Tweener {
  public:
    Tweener();
    static uint8_t tween(uint8_t prev, uint8_t next, float increment) {
      return prev + (int)((float)next - prev) * increment;
    }
};
#endif
