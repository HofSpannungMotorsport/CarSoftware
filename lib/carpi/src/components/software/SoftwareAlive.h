#ifndef SOFTWARE_ALIVE_H
#define SOFTWARE_ALIVE_H

#include "../interface/IAlive.h"

class SoftwareAlive : public IAlive {
    public:
        SoftwareAlive(id_sub_component_t componentSubId, IRegistry &registry)
        : _registry(registry) {
            setComponentSubId(componentSubId);
            setComponentType(COMPONENT_ALIVE);
            setObjectType(OBJECT_SOFTWARE);
        }

        void setAlive(bool alive) {
            // No implementation needed
        }

        bool getAlive() {
            return _alive;
        }

        virtual void setStatus(status_t status) {
            // No implementation needed
        }

        virtual status_t getStatus() {
            if (getAlive()) {
                return 0;
            } else {
                alive_error_type_t error = ALIVE_ERROR_DEAD;
                return error;
            }
        }

        virtual void receive(CarMessage &carMessage) {
            if (carMessage.getLength() == 2) {
                switch (carMessage.get(0)) {
                    case ALIVE_MESSAGE_SEND_ALIVE:
                        _aliveTicker.detach();
                        if (carMessage.get(1) == 0x1) {
                            _alive = true;
                            _aliveTicker.attach(callback(this, &SoftwareAlive::_noMessageReceived), _registry.getFloat(ALIVE_SIGNAL_REFRESH_RATE));
                        } else _alive = false;
                        break;
                }
            }
        }
    
    private:
        IRegistry &_registry;
        
        bool _alive;
        Ticker _aliveTicker;

        void _noMessageReceived() {
            // Probably dead... R.I.P.
            _alive = false;
        }
};

#endif // SOFTWARE_ALIVE_H