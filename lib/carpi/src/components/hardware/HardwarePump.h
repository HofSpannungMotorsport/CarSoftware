#ifndef HARDWAREPUMP_H
#define HARDWAREPUMP_H

#include "../interface/IPump.h"

class HardwarePump : public IPump {
    public:
        HardwarePump(PinName pwmPort, PinName enablePort) : _pwmPort(pwmPort), _enablePort(enablePort) {
            _enablePort.write(0);
            _pwmPort.write(0);

            _telegramTypeId = COOLING;
            _objectType = HARDWARE_OBJECT;
        }

        HardwarePump(PinName pwmPort, PinName enablePort, can_component_t componentId) : HardwarePump(pwmPort, enablePort) {
            _componentId = componentId;
        }

        virtual void setSpeed(pump_speed_t speed) {
            if (speed > 1) speed = 1;
            if (speed < 0) speed = 0;

            _pwmPort.write(speed);
        }

        virtual pump_speed_t getSpeed() {
            return _pwmPort.read();
        }

        virtual void setEnable(pump_enable_t enable) {
            _enablePort.write(enable);
        }

        virtual pump_enable_t getEnable() {
            return _enablePort.read();
        }

    protected:
        PwmOut _pwmPort;
        DigitalOut _enablePort;
};

#endif // HARDWAREPUMP_H