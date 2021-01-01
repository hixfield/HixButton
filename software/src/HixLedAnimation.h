#ifndef HixLedAnimation_h
#define HixLedAnimation_h

#include <Arduino.h>

enum class HixLedAnimation { off,
                             on,
                             slowBlink,
                             blink,
                             fastBlink,
                             fadeInOut };

//constants used for animation
static const unsigned long HIXLEDANIMATION_PERIOD_SLOW_BLINK_MS = 1000;
static const unsigned long HIXLEDANIMATION_PERIOD_BLINK_MS = 400;
static const unsigned long HIXLEDANIMATION_PERIOD_FAST_BLINK_MS = 100;
static const unsigned long HIXLEDANIMATION_PERIOD_FADEINOUT_MS = 1500;

#endif
