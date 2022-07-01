#ifndef HARDWAREPUMP_H
#define HARDWAREPUMP_H

#include "../interface/IPump.h"

#ifdef DISABLE_PUMP
    #warning "Pump Disabled!"
#endif

#define PUMP_MAX_SPEED 0.6 // %
#define PUMP_MAX_RUN_UP_TIME 20 // s
#define PUMP_RUN_UP_UPDATE_INTERVAL 0.1 // s

class HardwarePump : public IPump {
    public:
        HardwarePump(PinName pwmPort) : _pwmPort(pwmPort) {
            _pwmPort = 1; // -> pump pwm off

            setComponentType(COMPONENT_COOLING);
            setObjectType(OBJECT_HARDWARE);
        }

        HardwarePump(PinName pwmPort, id_sub_component_t componentSubId) : HardwarePump(pwmPort) {
            setComponentSubId(componentSubId);
        }

        virtual void setSpeed(pump_speed_t speed) {
            #ifdef DISABLE_PUMP
                speed = 0;
            #else
                if (speed > 1) speed = 1;
                if (speed < 0) speed = 0;

                _lastSetSpeed = speed;
            #endif

            if (_currentSpeed != speed && !_tickerAttached) {
                _tickerAttached = true;
                _runUpTicker.attach(callback(this, &HardwarePump::_updateSpeed), PUMP_RUN_UP_UPDATE_INTERVAL);
            }
        }

        virtual pump_speed_t getSpeed() {
            return _currentSpeed;
        }

    protected:
        PwmOut _pwmPort;

        pump_speed_t _currentSpeed = 0; // Important for the startup ramp
        pump_speed_t _lastSetSpeed = 0; // Last set Speed with setSpeed()

        Ticker _runUpTicker;
        bool _tickerAttached = false;

        void _updateSpeed() {
            if (_currentSpeed < _lastSetSpeed) {
                _currentSpeed += PUMP_RUN_UP_UPDATE_INTERVAL / PUMP_MAX_RUN_UP_TIME;

                if (_currentSpeed >= _lastSetSpeed) {
                    _runUpTicker.detach();
                    _tickerAttached = false;
                    _currentSpeed = _lastSetSpeed;
                }

            } else if (_currentSpeed > _lastSetSpeed) {
                _currentSpeed -= PUMP_RUN_UP_UPDATE_INTERVAL / PUMP_MAX_RUN_UP_TIME;

                if (_currentSpeed <= _lastSetSpeed) {
                    _runUpTicker.detach();
                    _tickerAttached = false;
                    _currentSpeed = _lastSetSpeed;
                }
            } else {
                _runUpTicker.detach();
                _tickerAttached = false;
            }
            
            _pwmPort.write(1 - (_currentSpeed * PUMP_MAX_SPEED));
        }
};

#endif // HARDWAREPUMP_H