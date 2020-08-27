#ifndef HARDWAREFAN_H
#define HARDWAREFAN_H

#include "../interface/IFan.h"

#define FAN_TURN_ON_DELAY 3 // s

class HardwareFan : public IFan {
    public:
        HardwareFan(PinName port) : _port(port) {
            _port = 0;

            setComponentType(COMPONENT_COOLING);
            setObjectType(OBJECT_HARDWARE);
        }

        HardwareFan(PinName port, id_sub_component_t componentSubId) : HardwareFan(port) {
            setComponentSubId(componentSubId);
        }

        virtual void setState(fan_state_t state) {
            _port = (bool)state;

            if (state == FAN_ON) {
                if (_fanSetpoint != FAN_ON) {
                    _fanSetpoint = FAN_ON;
                    _turnOnTicker.attach(callback(this, &HardwareFan::_updateOnState), FAN_TURN_ON_DELAY);
                }
            } else if (state == FAN_OFF) {
                _port.write(0);
            }
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
        Ticker _turnOnTicker;
        fan_state_t _fanSetpoint = FAN_OFF;

        void _updateOnState() {
            if (_fanSetpoint == FAN_ON) {
                _port.write(1);
            }

            _turnOnTicker.detach();
        }
};

#endif // HARDWAREFAN_H