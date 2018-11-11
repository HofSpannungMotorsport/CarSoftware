#ifndef SOFTWARELED_H
#define SOFTWARELED_H

#include "../interface/ILed.h"

class SoftwareLed : ILed {
    public:
        SoftwareLed() {
            _telegramTypeId = LED;
            _objectType = SOFTWARE_OBJECT;
        }

        SoftwareLed(can_component_t componentId)
            : SoftwareLed() {
            _componentId = componentId;
        }


        virtual void setState(led_state_t state) {
            _state = state;
        }

        virtual void setBrightness(led_brightness_t brightness) {
            _brightness = brightness;
        }

        virtual void setBlinking(led_blinking_t blinking) {
            _blinking = blinking;
        }

        // getters
        virtual led_state_t getState() {
            return _state;
        }

        virtual led_brightness_t getBrightness() {
            return _brightness;
        }
        
        virtual led_blinking_t getBlinking() {
            return _blinking;
        }

    private:
        led_state_t _state;
        led_brightness_t _brightness;
        led_blinking_t _blinking;
};

#endif