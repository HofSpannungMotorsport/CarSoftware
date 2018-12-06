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
            _objectType = HARDWARE_OBJECT;
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
        led_state_t _stateLast = LED_OFF;
        led_brightness_t _brightness = 1.0;
        led_blinking_t _mode = BLINKING_OFF;
        led_blinking_t _modeLast = BLINKING_OFF;

        bool _lastBlinkingState = false;

        struct _blinkingTime {
            float   slow = (1 / (float)BLINKING_SLOW_HZ) / 2,
                    normal = (1 / (float)BLINKING_NORMAL_HZ) / 2,
                    fast = (1 / (float)BLINKING_FAST_HZ) / 2;
        } _blinkingTime;

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
                _port.write(_brightness);
            } else if (_mode != _modeLast) {
                switch (_mode){
                    case BLINKING_SLOW:
                        _attachBlinkingTicker(_blinkingTime.slow);
                        break;
                
                    case BLINKING_NORMAL:
                        _attachBlinkingTicker(_blinkingTime.normal);
                        break;

                    case BLINKING_FAST:
                        _attachBlinkingTicker(_blinkingTime.fast);
                        break;

                    default:
                        _blinkingTicker.detach();
                        _port.write(_brightness);
                }
            }

            _modeLast = _mode;
            _stateLast = _state;
        }

        void _attachBlinkingTicker(float blinkingTime) {
            _blinkingTicker.detach();
            _port.write(_brightness);
            _lastBlinkingState = true;
            _blinkingTicker.attach(callback(this, &HardwareLedPwm::_blinkingLoop), blinkingTime);
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