#ifndef HARDWAREFAN_H
#define HARDWAREFAN_H

#include "../interface/IFan.h"

class HardwareFan : public IFan {
    public:
        HardwareFan(PinName port) : _port(port) {
            _port = 0;

            setComponentType(COMPONENT_COOLING);
            setObjectType(OBJECT_HARDWARE)
        }

        HardwareFan(PinName port, id_sub_component_t componentSubId) : HardwareFan(port) {
            setComponentSubId(componentSubId);
        }

        virtual void setState(fan_state_t state) {
            _port = (bool)state;
        }

        virtual fan_state_t getState() {
            if (_port) {
                return FAN_ON;
            } else {
                return FAN_OFF;
            }
        }

    protected:
        DigitalOut _port;
};

#endif // HARDWAREFAN_H