#ifndef HARDWAREPWMBUZZER_H
#define HARDWAREPWMBUZZER_H

#include "../interface/IBuzzer.h"

class HardwarePwmBuzzer : public IBuzzer {
    public:
        HardwarePwmBuzzer(PinName port, id_sub_component_t componentSubId, IRegistry &registry)
        : _port(port), _registry(registry) {
            setComponentSubId(componentSubId);
            setObjectType(OBJECT_HARDWARE);
        }

        virtual void setStatus(status_t status) {
            // No implementation needed
        }

        virtual status_t getStatus() {
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
            _current.hz = hz;
            _update();
        }

        virtual buzzer_hz_t getHz() {
            return _current.hz;
        }

    protected:
        IRegistry &_registry;
        bool _stdValuesLoaded = false;

        PwmOut _port;
        Ticker _ticker;
        status_t _status;

        struct _current {
            buzzer_state_t state = BUZZER_OFF;
            buzzer_hz_t hz;
            float highToLowRatio;

            struct beep {
                float valueHigh,
                      highToLowRatio,
                      switchTime;
                buzzer_beep_type_t type = BUZZER_MONO_TONE;
            } beep;
        } _current;

        struct _last {
            buzzer_state_t state = BUZZER_OFF;
            
            struct beep {
                bool state;
                buzzer_beep_type_t type = BUZZER_MONO_TONE;
            } beep;
        } _last;

        struct _time {
            float onOff,
                  highToLow,
                  fastHighToLow;
        } _time;

        float _pwmHighToLowRatio;

        void _loadStdValues() {
            _current.hz = _registry.getUInt16(STD_BUZZER_HIGH_HZ);
            _current.highToLowRatio = _registry.getFloat(STD_BUZZER_PWM_HIGH_TO_LOW_RATIO);

            _time.onOff = _registry.getFloat(STD_BUZZER_ON_OFF_TIME);
            _time.highToLow = _registry.getFloat(STD_BUZZER_HIGH_LOW_TIME);
            _time.fastHighToLow = _registry.getFloat(STD_BUZZER_FAST_HIGH_LOW_TIME);

            _pwmHighToLowRatio = _registry.getFloat(STD_BUZZER_PWM_HIGH_TO_LOW_RATIO);
        }

        void _beep() {
            float newValue = 0;

            if (_last.beep.state) {
                newValue = _current.beep.valueHigh * _current.beep.highToLowRatio;
            } else {
                newValue = _current.beep.valueHigh;
            }

            _last.beep.state = !_last.beep.state;

            _setHz(newValue);
        }

        void _setHz(buzzer_hz_t hz) {
            if (hz > 0) {
                _port.write(_current.highToLowRatio);
                _port.period(1/hz);
            } else {
                _port.write(0);
            }
        }

        void _startBeep() {
            _setHz(_current.beep.valueHigh);
            _last.beep.state = false;
            _beep();
            _ticker.attach(callback(this, &HardwarePwmBuzzer::_beep), _current.beep.switchTime);
        }

        void _update() {
            if (!_stdValuesLoaded) {
                _loadStdValues();
                _stdValuesLoaded = true;
            }

            if (_current.state == BUZZER_OFF) {
                _port.write(0);
                _ticker.detach();
                _current.beep.switchTime = 0;
            } else if (_current.state == BUZZER_ON) {
                if (_current.beep.type == BUZZER_MONO_TONE) {
                    _ticker.detach();
                    _setHz(_current.hz);
                } else if (_current.beep.type == BUZZER_BEEP_ON_BEEP_OFF) {
                    _current.beep.valueHigh = _current.hz;
                    _current.beep.highToLowRatio = 0;
                    if (_time.onOff != _current.beep.switchTime) {
                        _ticker.detach();
                        _current.beep.switchTime = _time.onOff;
                        _startBeep();
                    }
                } else if (_current.beep.type == BUZZER_BEEP_HIGH_BEEP_LOW) {
                    _current.beep.valueHigh = _current.hz;
                    _current.beep.highToLowRatio = _pwmHighToLowRatio;
                    if (_time.highToLow != _current.beep.switchTime) {
                        _ticker.detach();
                        _current.beep.switchTime = _time.highToLow;
                        _startBeep();
                    }
                } else if (_current.beep.type == BUZZER_BEEP_FAST_HIGH_LOW) {
                    _current.beep.valueHigh = _current.hz;
                    _current.beep.highToLowRatio = _pwmHighToLowRatio;
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

            _last.state = _current.state;
            _last.beep.type = _current.beep.type;
        }
};

#endif // HARDWAREPWMBUZZER_H