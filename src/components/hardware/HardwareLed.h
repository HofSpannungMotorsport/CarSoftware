#ifndef HARDWARELED_H
#define HARDWARELED_H
#include "mbed.h"
#include "ILed.h"


class HardwareLed : public ILed
{
    private:
        DigitalOut *outPin;
    public:
        HardwareLed(PinName pin)
        {
           outPin = new DigitalOut(pin);
        }
        virtual ~HardwareLed()
        {
            delete outPin;
        }

        virtual void setState(led_state_t state)
        {
            outPin->write(state);
        }
        virtual void setBrightness(led_brightness_t brightness)
        {

        }
        virtual void setBlinking(led_blinking_t)
        {

        }

        // getters
        virtual led_state_t getState()
        {
            return (led_state_t)outPin->read();
        }

        virtual led_brightness_t getBrightness()
        {
            return 3; // brightest
        }
        
        virtual led_blinking_t getBlinking()
        {
            return BLINKING_OFF;
        }
        
};

#endif