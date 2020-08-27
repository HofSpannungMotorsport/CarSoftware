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
        HardwarePump(PinName pwmPort, PinName enablePort) : _pwmPort(pwmPort), _enablePort(enablePort) {
            _enablePort = 0;
            _pwmPort = 1; // -> pump pwm off

            setComponentType(COMPONENT_COOLING);
            setObjectType(OBJECT_HARDWARE);
        }

        HardwarePump(PinName pwmPort, PinName enablePort, id_sub_component_t componentSubId) : HardwarePump(pwmPort, enablePort) {
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

            if (_enabled) {
                _runUpTicker.attach(callback(this, &HardwarePump::_updateSpeed), PUMP_RUN_UP_UPDATE_INTERVAL);
            } else {
                _pwmPort.write(1);
                _currentSpeed = 0;
            }
        }

        virtual pump_speed_t getSpeed() {
            return _currentSpeed;
        }

        virtual void setEnable(bool enable) {
            #ifndef DISABLE_PUMP
                if (!enable) {
                    _pwmPort.write(1);
                    _currentSpeed = 0;
                }

                _enablePort.write(enable);
                _enabled = enable;

                if (enable) {
                    setSpeed(_lastSetSpeed);
                }
            #endif
        }

        virtual bool getEnable() {
            return _enabled;
        }

    protected:
        PwmOut _pwmPort;
        DigitalOut _enablePort;

        bool _enabled = false;
        pump_speed_t _currentSpeed = 0; // Important for the startup ramp
        pump_speed_t _lastSetSpeed = 0; // Last set Speed with setSpeed()

        Ticker _runUpTicker;

        void _updateSpeed() {
            _currentSpeed += PUMP_RUN_UP_UPDATE_INTERVAL / PUMP_MAX_RUN_UP_TIME;

            if (_currentSpeed > _lastSetSpeed) {
                _runUpTicker.detach();
                _currentSpeed = _lastSetSpeed;
            }
            
            _pwmPort.write(1 - (_currentSpeed * PUMP_MAX_SPEED));
        }
};

#endif // HARDWAREPUMP_H