#ifndef HARDWAREPUMP_H
#define HARDWAREPUMP_H

#include "../interface/IPump.h"

class HardwarePump : public IPump {
    public:
        HardwarePump(PinName port) : _port(port) {
            _port.write(0);

            _telegramTypeId = COOLING;
            _objectType = HARDWARE_OBJECT;
        }

        HardwarePump(PinName port, can_component_t componentId) : HardwarePump(PinName port) {
            _componentId = componentId;
        }

        virtual void setSpeed(pump_speed_t speed) {
            if (speed > 1) speed = 1;
            if (speed < 0) speed = 0;

            _port.write(speed);
        }

        virtual pump_speed_t getSpeed() {
            return _port.read();
        }

    protected:
        PwmOut _port;
};

#endif // HARDWAREPUMP_H