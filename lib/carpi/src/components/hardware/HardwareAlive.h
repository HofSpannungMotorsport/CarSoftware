#ifndef HARDWARE_ALIVE_H
#define HARDWARE_ALIVE_H

#include "../interface/IAlive.h"

class HardwareAlive : public IAlive {
    public:
        HardwareAlive(id_sub_component_t componentSubId) {
            setComponentType(COMPONENT_ALIVE);
            setComponentSubId(componentSubId);
            setObjectType(OBJECT_HARDWARE);
        }

        HardwareAlive(id_sub_component_t componentSubId, PinName port)
        : HardwareAlive(componentSubId) {
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
                if (_alive) {
                    _sendCommand(ALIVE_MESSAGE_SEND_ALIVE, 0x1, SEND_PRIORITY_ALIVE, STD_ALIVE_MESSAGE_TIMEOUT);
                }
            }
        }

        bool getAlive() {
            return _alive;
        }

        alive_status_t getStatus() {
            if (_alive) {
                return 0;
            } else {
                alive_error_type_t error = ALIVE_ERROR_DEAD;
                return error;
            }
        }

    private:
        static PinName _port;
        static bool _portSet;
        static bool _alive;

        void _setOutput(bool alive) {
            static DigitalOut port(_port);

            if (alive) {
                port = 1;
            } else {
                port = 0;
            }
        }
};

#endif // HARDWARE_ALIVE_H