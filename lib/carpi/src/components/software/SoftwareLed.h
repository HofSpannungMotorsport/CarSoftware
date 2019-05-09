#ifndef SOFTWARELED_H
#define SOFTWARELED_H

#include "../interface/ILed.h"

class SoftwareLed : public ILed {
    public:

        SoftwareLed(id_sub_component_t componentSubId) {
            setComponentType(COMPONENT_LED);
            setObjectType(OBJECT_SOFTWARE);
            setComponentSubId(componentSubId);
        }

        virtual void setState(led_state_t state) {
            if (_syncerAttached)
                _sendCommand(LED_MESSAGE_COMMAND_SET_STATE, state, SEND_PRIORITY_LED, STD_LED_MESSAGE_TIMEOUT, IS_NOT_DROPABLE);
        }

        virtual void setBrightness(led_brightness_t brightness) {
            if (brightness > 1.0) brightness = 1.0;
            else if (brightness < 0.0) brightness = 0.0;

            if (_syncerAttached)
                _sendCommand(LED_MESSAGE_COMMAND_SET_BRIGHTNESS, ((led_brightness_t)brightness * 255.0), SEND_PRIORITY_LED, STD_LED_MESSAGE_TIMEOUT, IS_NOT_DROPABLE);
        }

        virtual void setBlinking(led_blinking_t blinking) {
            if (_syncerAttached)
                _sendCommand(LED_MESSAGE_COMMAND_SET_BLINKING, blinking, SEND_PRIORITY_LED, STD_LED_MESSAGE_TIMEOUT, IS_NOT_DROPABLE);
        }

        virtual void receive(CarMessage &carMessage) {
            // No implementation needed yet
        }
};

#endif