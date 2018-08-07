#ifndef LED_H
#define LED_H
#include "mbed.h"
#include "ILed.h"


class Led : public ILed
{
    private:
        DigitalOut *outPin;
    public:
        Led(PinName pin)
        {
           outPin = new DigitalOut(pin);
        }
        virtual ~Led()
        {
            delete outPin;
        }

        virtual void setState(led_state_t state)
        {
            outPin->write(state);
        }
        virtual void setBrightness(uint8_t brightness)
        {

        }
        virtual void setBlinking(led_blinking_t)
        {

        }
};

#endif