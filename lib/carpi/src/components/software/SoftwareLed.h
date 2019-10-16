#ifndef SOFTWARELED_H
#define SOFTWARELED_H

#include "../interface/ILed.h"

class SoftwareLed : public ILed {
    public:
        SoftwareLed(id_sub_component_t componentSubId) {
            setObjectType(OBJECT_SOFTWARE);
            setComponentSubId(componentSubId);
        }

        virtual void setState(led_state_t state) {
            if (_syncerAttached)
                _sendCommand(LED_MESSAGE_COMMAND_SET_STATE, state, SEND_PRIORITY_LED, IS_NOT_DROPABLE);
            
            _state = state;
        }

        virtual void setBrightness(led_brightness_t brightness) {
            if (brightness > 1.0) brightness = 1.0;
            else if (brightness < 0.0) brightness = 0.0;

            if (_syncerAttached)
                _sendCommand(LED_MESSAGE_COMMAND_SET_BRIGHTNESS, ((led_brightness_t)brightness * 255.0), SEND_PRIORITY_LED, IS_NOT_DROPABLE);
            
            _brightness = brightness;
        }

        virtual void setBlinking(led_blinking_t blinking) {
            if (_syncerAttached)
                _sendCommand(LED_MESSAGE_COMMAND_SET_BLINKING, blinking, SEND_PRIORITY_LED, IS_NOT_DROPABLE);
            
            _blinking = blinking;
        }

        virtual led_state_t getState() {
            return _state;
        }

        virtual led_brightness_t getBrightness() {
            return _brightness;
        }

        virtual led_blinking_t getBlinking() {
            return _blinking;
        }

        virtual void receive(CarMessage &carMessage) {
            // No implementation needed yet
        }

        virtual void setStatus(status_t status) {
            // No implementation needed
        }

        virtual status_t getStatus() {
            // No implementation needed
            return 0;
        }

    protected:
        led_state_t _state;
        led_brightness_t _brightness;
        led_blinking_t _blinking;
};

#endif