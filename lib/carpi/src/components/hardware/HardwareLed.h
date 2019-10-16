#ifndef HARDWARELED_H
#define HARDWARELED_H

#include "../interface/ILed.h"

class HardwareLed : public ILed {
    public:
        HardwareLed(PinName port, id_sub_component_t componentSubId, IRegistry &registry)
            : _port(port), _registry(registry) {
            setComponentSubId(componentSubId);
            setObjectType(OBJECT_HARDWARE);
        }

        // setters
        virtual void setState(led_state_t state) {
            _state = state;
            _refresh();
        }

        virtual void setBrightness(led_brightness_t brightness) {
            // No implementation for non-dimmable LED
        }

        virtual void setBlinking(led_blinking_t mode) {
            _mode = mode;
            _refresh();
        }

        virtual led_state_t getState() {
            return _state;
        }

        virtual led_brightness_t getBrightness() {
            return 1;
        }

        virtual led_blinking_t getBlinking() {
            return _mode;
        }

        virtual void receive(CarMessage &carMessage) {
            for (car_sub_message_t &subMessage : carMessage.subMessages) {
                switch (subMessage.data[0]) {
                    case LED_MESSAGE_COMMAND_SET_STATE:
                        setState((led_state_t)subMessage.data[1]);
                        break;
                    
                    case LED_MESSAGE_COMMAND_SET_BLINKING:
                        setBlinking((led_blinking_t)subMessage.data[1]);
                        break;
                    
                    case LED_MESSAGE_COMMAND_SET_BRIGHTNESS:
                        setBrightness((led_brightness_t)subMessage.data[1] / 255.0);
                        break;
                }
            }
        }

        virtual void setStatus(status_t status) {
            // No implementation needed
        }

        virtual status_t getStatus() {
            // No implementation needed
            return 0;
        }

    protected:
        IRegistry &_registry;

        DigitalOut _port;

        led_state_t _state = LED_OFF;
        led_state_t _stateLast = LED_OFF;

        led_blinking_t _mode = BLINKING_OFF;
        led_blinking_t _modeLast = BLINKING_OFF;

        bool _lastBlinkingState = false;

        Ticker _blinkingTicker;

        void _refresh() {
            if (_state == LED_OFF) {
                _blinkingTicker.detach();
                _stateLast = LED_OFF;
                _modeLast = BLINKING_OFF;
                _port = LED_OFF;
                return;
            }

            if (_mode == BLINKING_OFF) {
                _blinkingTicker.detach();
                _port = LED_ON;
            } else if (_mode != _modeLast) {
                switch (_mode){
                    case BLINKING_SLOW:
                        _attachBlinkingTicker(1 / _registry.getFloat(LED_BLINKING_SLOW_HZ) / 2);
                        break;
                
                    case BLINKING_NORMAL:
                        _attachBlinkingTicker(1 / _registry.getFloat(LED_BLINKING_NORMAL_HZ) / 2);
                        break;

                    case BLINKING_FAST:
                        _attachBlinkingTicker(1 / _registry.getFloat(LED_BLINKING_FAST_HZ) / 2);
                        break;

                    default:
                        _blinkingTicker.detach();
                        _port = LED_ON;
                }
            }

            _modeLast = _mode;
            _stateLast = _state;
        }

        void _attachBlinkingTicker(float blinkingTime) {
            _blinkingTicker.detach();
            _port = LED_ON;
            _lastBlinkingState = true;
            _blinkingTicker.attach(callback(this, &HardwareLed::_blinkingLoop), blinkingTime);
        }

        void _blinkingLoop() {
            _lastBlinkingState = !_lastBlinkingState;
            _port = _lastBlinkingState;
        }
};

#endif