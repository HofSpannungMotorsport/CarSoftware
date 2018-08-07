#ifndef ILED_H
#define ILED_H

#include <stdint.h>

enum led_state_t
{
    STATE_ON = 0x1,
    STATE_OFF = 0x0

};

enum led_blinking_t
{
    BLINKING_OFF = 0x0,
    BLINKING_SLOW = 0x1,
    BLINKING_NORMAL = 0x2,
    BLINKING_FAST = 0x3
};


class ILed
{
    public:
        virtual void setState(led_state_t state) = 0;
        virtual void setBrightness(uint8_t brightness) = 0;
        virtual void setBlinking(led_blinking_t mode) = 0;
};

#endif