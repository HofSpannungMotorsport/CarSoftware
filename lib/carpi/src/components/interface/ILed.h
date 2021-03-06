#ifndef ILED_H
#define ILED_H

#include "ICommunication.h"

enum led_state_t : uint8_t {
    LED_ON = 0x1,
    LED_OFF = 0x0
};

enum led_blinking_t : uint8_t {
    BLINKING_OFF = 0x0,
    BLINKING_SLOW = 0x1,
    BLINKING_NORMAL = 0x2,
    BLINKING_FAST = 0x3
};

typedef float led_brightness_t;


class ILed : public ICommunication {
    public:
        // setters
        virtual void setState(led_state_t state) = 0;
        virtual void setBrightness(led_brightness_t brightness) = 0;
        virtual void setBlinking(led_blinking_t mode) = 0;

        // getters
        virtual led_state_t getState() = 0;
        virtual led_brightness_t getBrightness() = 0;
        virtual led_blinking_t getBlinking() = 0;
        virtual bool getSentConfigurationChanged() = 0;
};

#endif