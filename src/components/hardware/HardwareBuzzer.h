#ifndef HARDWAREBUZZER_H
#define HARDWAREBUZZER_H

#include "mbed.h"
#include "../interface/IBuzzer.h"
#include "../../can/can_ids.h"

#define STD_BUZZER_ON_OFF_TIME  0.5 // s
#define STD_BUZZER_FAST_HIGH_LOW_TIME 0.075 // s

class HardwareBuzzer : public IBuzzer {
    public:
        HardwareBuzzer(PinName port)
            : _port(port) {}
        
        HardwareBuzzer(PinName port, can_component_t componentId)
            : HardwareBuzzer(port) {
            _componentId = componentId;
        }

        virtual void setStatus(buzzer_status_t status) {
            // No implementation needed
        }

        virtual buzzer_status_t getStatus() {
            return _status;
        }

        virtual void setState(buzzer_state_t state) {
            _current.state = state;
            _update();
        }

        virtual buzzer_state_t getState() {
            return _current.state;
        }

        virtual void setBeep(buzzer_beep_type_t beep) {
            _current.beep.type = beep;
            _update();
        }

        virtual buzzer_beep_type_t getBeep() {
            return _current.beep.type;
        }

        virtual void setHz(buzzer_hz_t hz) {
            // No implementation needed
        }

        virtual buzzer_hz_t getHz() {
            // No implementation needed
            return 0;
        }

    protected:
        DigitalOut _port;
        Ticker _ticker;
        buzzer_status_t _status;

        struct _current {
            buzzer_state_t state = BUZZER_OFF;

            struct beep {
                float switchTime;
                buzzer_beep_type_t type = BUZZER_MONO_TONE;
            } beep;
        } _current;

        bool _lastBeepState;

        struct _time {
            float onOff = STD_BUZZER_ON_OFF_TIME,
                  fastHighToLow = STD_BUZZER_FAST_HIGH_LOW_TIME;
        } _time;

        void _beep() {
            _lastBeepState = !_lastBeepState;
            _port = _lastBeepState;
        }

        void _startBeep() {
            _lastBeepState = false;
            _beep();
            _ticker.attach(callback(this, &HardwareBuzzer::_beep), _current.beep.switchTime);
        }

        void _update() {
            if (_current.state == BUZZER_OFF) {
                _port.write(0);
                _ticker.detach();
                _current.beep.switchTime = 0;
            } else if (_current.state == BUZZER_ON) {
                if (_current.beep.type == BUZZER_MONO_TONE) {
                    _ticker.detach();
                    _port = 1;
                } else if (_current.beep.type == BUZZER_BEEP_ON_BEEP_OFF || _current.beep.type == BUZZER_BEEP_HIGH_BEEP_LOW) {
                    if (_time.onOff != _current.beep.switchTime) {
                        _ticker.detach();
                        _current.beep.switchTime = _time.onOff;
                        _startBeep();
                    }
                } else if (_current.beep.type == BUZZER_BEEP_FAST_HIGH_LOW) {
                    if (_time.fastHighToLow != _current.beep.switchTime) {
                        _ticker.detach();
                        _current.beep.switchTime = _time.fastHighToLow;
                        _startBeep();
                    }
                } else {
                    buzzer_error_type_t errorWrongBeep = BUZZER_WRONG_BEEP;
                    _status |= errorWrongBeep;
                }
            } else {
                buzzer_error_type_t errorWrongState = BUZZER_WRONG_STATE;
                _status |= errorWrongState;
            }
        }
};

#endif // HARDWAREBUZZER_H