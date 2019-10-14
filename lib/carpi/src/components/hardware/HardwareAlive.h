#ifndef HARDWARE_ALIVE_H
#define HARDWARE_ALIVE_H

#include "../interface/IAlive.h"

class HardwareAlive : public IAlive {
    public:
        HardwareAlive(id_sub_component_t componentSubId, IRegistry &registry)
        : _registry(registry) {
            setComponentType(COMPONENT_ALIVE);
            setComponentSubId(componentSubId);
            setObjectType(OBJECT_HARDWARE);
            _portSet = false;
            _alive = false;
        }

        HardwareAlive(id_sub_component_t componentSubId, PinName port, IRegistry &registry)
        : HardwareAlive(componentSubId, registry) {
            _port = port;
            _portSet = true;
            _setOutput(false);
        }

        void setAlive(bool alive) {
            _alive = alive;

            if (_portSet) {
                _setOutput(alive);
            }

            if (_syncerAttached) {
                _aliveTicker.detach();

                _updateAlive();

                _aliveTicker.attach(callback(this, &HardwareAlive::_updateAlive), _registry.getFloat(ALIVE_SIGNAL_SEND_RATE));
            }
        }

        bool getAlive() {
            return _alive;
        }

        virtual void setStatus(status_t status) {
            // No implementation needed
        }

        virtual status_t getStatus() {
            if (_alive) {
                return 0;
            } else {
                alive_error_type_t error = ALIVE_ERROR_DEAD;
                return error;
            }
        }

        virtual void receive(CarMessage &carMessage) {
            // No implementation needed
        }

    private:
        IRegistry &_registry;

        PinName _port;
        bool _portSet;
        bool _alive;

        Ticker _aliveTicker;

        void _setOutput(bool alive) {
            static DigitalOut port(_port);

            if (alive) {
                port = 1;
            } else {
                port = 0;
            }
        }

        void _updateAlive() {
            if (_alive) {
                _sendCommand(ALIVE_MESSAGE_SEND_ALIVE, 0x1, SEND_PRIORITY_ALIVE, IS_NOT_DROPABLE);
            } else {
                _sendCommand(ALIVE_MESSAGE_SEND_ALIVE, 0x0, SEND_PRIORITY_ALIVE, IS_NOT_DROPABLE);
            }
        }
};

#endif // HARDWARE_ALIVE_H