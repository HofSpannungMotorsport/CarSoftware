#ifndef SOFTWARELED_H
#define SOFTWARELED_H

#include "../interface/ILed.h"

class SoftwareLed : ILed {
    public:
        SoftwareLed() {
            setComponentType(COMPONENT_LED);
            setObjectType(OBJECT_SOFTWARE);
        }

        SoftwareLed(id_sub_component_t componentSubId)
            : SoftwareLed() {
            setComponentSubId(componentSubId);
        }


        virtual void setState(led_state_t state) {
            if (_state != state) {
                _sentConfigurationChanged = true;
            }

            _state = state;
        }

        virtual void setBrightness(led_brightness_t brightness) {
            if (_brightness != brightness) {
                _sentConfigurationChanged = true;
            }

            _brightness = brightness;
        }

        virtual void setBlinking(led_blinking_t blinking) {
            if (_blinking != blinking) {
                _sentConfigurationChanged = true;
            }

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

        virtual bool getSentConfigurationChanged() {
            if (_sentConfigurationChanged) {
                _sentConfigurationChanged = false;
                return true;
            }

            return false;
        }

    private:
        led_state_t _state;
        led_brightness_t _brightness;
        led_blinking_t _blinking;
        bool _sentConfigurationChanged = true;
};

#endif