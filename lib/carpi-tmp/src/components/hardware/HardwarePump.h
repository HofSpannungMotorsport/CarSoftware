#ifndef HARDWAREPUMP_H
#define HARDWAREPUMP_H

#include "../interface/IPump.h"

class HardwarePump : public IPump {
    public:
        HardwarePump(PinName pwmPort, PinName enablePort) : _pwmPort(pwmPort), _enablePort(enablePort) {
            _enablePort = 0;
            _pwmPort = 0;

            setComponentType(COMPONENT_COOLING_PUMP);
            setObjectType(OBJECT_HARDWARE);
        }

        HardwarePump(PinName pwmPort, PinName enablePort, id_sub_component_t componentSubId) : HardwarePump(pwmPort, enablePort) {
            setComponentSubId(componentSubId);
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