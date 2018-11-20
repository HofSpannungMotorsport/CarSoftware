#ifndef HARDWAREFAN_H
#define HARDWAREFAN_H

#include "../interface/IFan.h"

class HardwareFan : public IFan {
    public:
        HardwareFan(PinName port) : _port(port) {
            _port = 0;

            _telegramTypeId = COOLING;
            _objectType = HARDWARE_OBJECT;
        }

        HardwareFan(PinName port, can_component_t componentId) : HardwareFan(PinName port) {
            _componentId = componentId;
        }

        virtual void setState(fan_state_t state) {
            _port = (bool)state;
        }

        virtual fan_state_t getState() {
            return (fan_state_t)_port;
        }

    protected:
        DigitalOut _port;
};

#endif // HARDWAREFAN_H