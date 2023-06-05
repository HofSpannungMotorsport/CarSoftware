#ifndef HARDWAREPWMBUZZER_H
#define HARDWAREPWMBUZZER_H

#include "../interface/IBuzzer.h"

#define STD_BUZZER_HIGH_HZ 300

#define STD_BUZZER_ON_OFF_TIME 0.5   // s
#define STD_BUZZER_HIGH_LOW_TIME 0.6 // s

#ifdef STD_BUZZER_FAST_HIGH_LOW_TIME
#undef STD_BUZZER_FAST_HIGH_LOW_TIME
#endif

#define STD_BUZZER_FAST_HIGH_LOW_TIME 0.15    // s
#define STD_BUZZER_BEEP_HIGH_TO_LOW_RATIO 0.6 // -> Low = 60% of High

#define STD_BUZZER_PWM_HIGH_TO_LOW_RATIO 0.5 // -> 50% of time High

#define RTD_BEEP_FREQUENCY 1000 // Hz
#define RTD_BEEP_DURATION 1.5   // s

class HardwarePwmBuzzer : public IBuzzer
{
public:
    HardwarePwmBuzzer(PinName port)
        : _port(port)
    {
        setComponentType(COMPONENT_BUZZER);
        setObjectType(OBJECT_HARDWARE);
    }

    HardwarePwmBuzzer(PinName port, id_sub_component_t componentSubId)
        : HardwarePwmBuzzer(port)
    {
        setComponentSubId(componentSubId);
    }

    virtual void setStatus(buzzer_status_t status)
    {
        // No implementation needed
    }

    virtual buzzer_status_t getStatus()
    {
        return _status;
    }

    virtual void setState(buzzer_state_t state)
    {
        _current.state = state;
        _update();
    }

    virtual buzzer_state_t getState()
    {
        return _current.state;
    }

    virtual void setBeep(buzzer_beep_type_t beep)
    {
        _current.beep.type = beep;
        _update();
    }

    virtual buzzer_beep_type_t getBeep()
    {
        return _current.beep.type;
    }

    virtual void setHz(buzzer_hz_t hz)
    {
        _current.hz = hz;
        _update();
    }

    virtual buzzer_hz_t getHz()
    {
        return _current.hz;
    }

    void playTone(float frequency, float duration)
    {
        _port.period(1.0 / frequency);
        _port = 0.1; // 10% duty cycle, otherwise it's too loud
        wait(duration / 1000);
        _port = 0.0; // Turn off the output
    }

    void playRtdTone()
    {
        _port.period(1.0 / RTD_BEEP_FREQUENCY);
        _port = 1; // 100% duty cycle, to be loud
        wait(RTD_BEEP_DURATION);
        _port = 0.0; // Turn off the output
    }

protected:
    PwmOut _port;
    Ticker _ticker;
    buzzer_status_t _status;

    struct _current
    {
        buzzer_state_t state = BUZZER_OFF;
        buzzer_hz_t hz = STD_BUZZER_HIGH_HZ;
        float highToLowRatio = STD_BUZZER_PWM_HIGH_TO_LOW_RATIO;

        struct beep
        {
            float valueHigh,
                highToLowRatio,
                switchTime;
            buzzer_beep_type_t type = BUZZER_MONO_TONE;
        } beep;
    } _current;

    struct _last
    {
        buzzer_state_t state = BUZZER_OFF;

        struct beep
        {
            bool state;
            buzzer_beep_type_t type = BUZZER_MONO_TONE;
        } beep;
    } _last;

    struct _time
    {
        float onOff = STD_BUZZER_ON_OFF_TIME,
              highToLow = STD_BUZZER_HIGH_LOW_TIME,
              fastHighToLow = STD_BUZZER_FAST_HIGH_LOW_TIME;
    } _time;

    void _beep()
    {
        float newValue = 0;

        if (_last.beep.state)
        {
            newValue = _current.beep.valueHigh * _current.beep.highToLowRatio;
        }
        else
        {
            newValue = _current.beep.valueHigh;
        }

        _last.beep.state = !_last.beep.state;

        _setHz(newValue);
    }

    void _setHz(buzzer_hz_t hz)
    {
        if (hz > 0)
        {
            _port.write(_current.highToLowRatio);
            _port.period(1 / hz);
        }
        else
        {
            _port.write(0);
        }
    }

    void _startBeep()
    {
        _setHz(_current.beep.valueHigh);
        _last.beep.state = false;
        _beep();
        _ticker.attach(callback(this, &HardwarePwmBuzzer::_beep), _current.beep.switchTime);
    }

    void _update()
    {
        if (_current.state == BUZZER_OFF)
        {
            _port.write(0);
            _ticker.detach();
            _current.beep.switchTime = 0;
        }
        else if (_current.state == BUZZER_ON)
        {
            if (_current.beep.type == BUZZER_MONO_TONE)
            {
                _ticker.detach();
                _setHz(_current.hz);
            }
            else if (_current.beep.type == BUZZER_BEEP_ON_BEEP_OFF)
            {
                _current.beep.valueHigh = _current.hz;
                _current.beep.highToLowRatio = 0;
                if (_time.onOff != _current.beep.switchTime)
                {
                    _ticker.detach();
                    _current.beep.switchTime = _time.onOff;
                    _startBeep();
                }
            }
            else if (_current.beep.type == BUZZER_BEEP_HIGH_BEEP_LOW)
            {
                _current.beep.valueHigh = _current.hz;
                _current.beep.highToLowRatio = STD_BUZZER_BEEP_HIGH_TO_LOW_RATIO;
                if (_time.highToLow != _current.beep.switchTime)
                {
                    _ticker.detach();
                    _current.beep.switchTime = _time.highToLow;
                    _startBeep();
                }
            }
            else if (_current.beep.type == BUZZER_BEEP_FAST_HIGH_LOW)
            {
                _current.beep.valueHigh = _current.hz;
                _current.beep.highToLowRatio = STD_BUZZER_BEEP_HIGH_TO_LOW_RATIO;
                if (_time.fastHighToLow != _current.beep.switchTime)
                {
                    _ticker.detach();
                    _current.beep.switchTime = _time.fastHighToLow;
                    _startBeep();
                }
            }
            else
            {
                buzzer_error_type_t errorWrongBeep = BUZZER_WRONG_BEEP;
                _status |= errorWrongBeep;
            }
        }
        else
        {
            buzzer_error_type_t errorWrongState = BUZZER_WRONG_STATE;
            _status |= errorWrongState;
        }

        _last.state = _current.state;
        _last.beep.type = _current.beep.type;
    }
};

#endif // HARDWAREPWMBUZZER_H