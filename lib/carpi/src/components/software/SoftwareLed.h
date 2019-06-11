#ifndef SOFTWARELED_H
#define SOFTWARELED_H

#include "../interface/ILed.h"

class SoftwareLed : public ILed {
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

        virtual message_build_result_t buildMessage(CarMessage &carMessage) {
            car_sub_message_t subMessage;

            subMessage.data[0] = (this->getState() & 0x1) << 7;
            subMessage.data[0] |= ((uint8_t)(this->getBrightness() * 0x1F) & 0x1F) << 2;
            subMessage.data[0] |= ((this->getBlinking() & 0x3));
            subMessage.length = 1;
            carMessage.addSubMessage(subMessage);

            return MESSAGE_BUILD_OK;
        }

        virtual message_parse_result_t parseMessage(CarMessage &carMessage) {
            // No implementation needed yet
            return MESSAGE_PARSE_ERROR;
        }

    private:
        led_state_t _state;
        led_brightness_t _brightness;
        led_blinking_t _blinking;
        bool _sentConfigurationChanged = true;
};

#endif