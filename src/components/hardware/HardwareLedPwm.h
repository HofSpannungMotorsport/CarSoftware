#ifndef HARDWARELED_H
#define HARDWARELED_H

#include "mbed.h"
#include "../interface/ILed.h"

#define BLINKING_SLOW_HZ    0.66
#define BLINKING_NORMAL_HZ  3
#define BLINKING_FAST_HZ    6

class HardwareLedPwm : public ILed {
    public:
        HardwareLedPwm(PinName port)
            : _port(port) {
            _telegramTypeId = LED;
            _objectType = HARDWARE;
        }

        HardwareLedPwm(PinName port, can_component_t componentId)
            : HardwareLedPwm(port) {
            _componentId = componentId;
        }

        // setters
        virtual void setState(led_state_t state) {
            _state = state;
            _refresh();
        }

        virtual void setBrightness(led_brightness_t brightness) {
            _brightness = brightness;

            if (_brightness > 1.0)
                _brightness = 1.0;

            if (_brightness < 0)
                _brightness = 0;

            _refresh();
        }

        virtual void setBlinking(led_blinking_t mode) {
            _mode = mode;
            _refresh();
        }

        // getters
        virtual led_state_t getState() {
            return _state;
        }

        virtual led_brightness_t getBrightness() {
            return _brightness;
        }

        virtual led_blinking_t getBlinking() {
            return _mode;
        }

    protected:
        PwmOut _port;
        led_state_t _state = LED_OFF;
        led_brightness_t _brightness = 1.0;
        led_blinking_t _mode = BLINKING_OFF;

        bool _lastBlinkingState = false;

        struct _blinkingTime {
            float   slow = (1 / (float)BLINKING_SLOW_HZ) / 2,
                    normal = (1 / (float)BLINKING_NORMAL_HZ) / 2,
                    fast = (1 / (float)BLINKING_FAST_HZ) / 2;
        } _blinkingTime;

        Ticker _blinkingTicker;

        void _refresh() {
            // Need to turn off the ticker first
            _blinkingTicker.detach();

            if (_state == LED_OFF) {
                _port.write(LED_OFF);
                return;
            }

            _port.write(_brightness);

            if (_mode != BLINKING_OFF) {
                float blinkingTime;
                switch (_mode){
                    case BLINKING_SLOW:
                        blinkingTime = _blinkingTime.slow;
                        break;
                
                    case BLINKING_NORMAL:
                        blinkingTime = _blinkingTime.normal;
                        break;

                    case BLINKING_FAST:
                        blinkingTime = _blinkingTime.fast;
                        break;

                    default:
                        return;
                }

                _lastBlinkingState = true;
                _blinkingTicker.attach(callback(this, &HardwareLedPwm::_blinkingLoop), blinkingTime);
            }
        }

        void _blinkingLoop() {
            led_brightness_t newOutput;
            if (_lastBlinkingState) {
                newOutput = 0;
            } else {
                newOutput = _brightness;
            }
            _lastBlinkingState = !_lastBlinkingState;

            _port.write(newOutput);
        }
};

#endif