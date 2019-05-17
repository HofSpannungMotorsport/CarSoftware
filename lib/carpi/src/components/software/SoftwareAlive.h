#ifndef SOFTWARE_ALIVE_H
#define SOFTWARE_ALIVE_H

#include "../interface/IAlive.h"

class SoftwareAlive : public IAlive {
    public:
        SoftwareAlive(id_sub_component_t componentSubId) {
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
            for(car_sub_message_t &subMessage : carMessage.subMessages) {
                if (subMessage.length == 2) {
                    switch (subMessage.data[0]) {
                        case ALIVE_MESSAGE_SEND_ALIVE:
                            _aliveTicker.detach();
                            if (subMessage.data[1] == 0x1) {
                                _alive = true;
                                _aliveTicker.attach(callback(this, &SoftwareAlive::_noMessageReceived), (float)STD_ALIVE_SIGNAL_REFRESH_RATE);
                            } else _alive = false;
                            break;
                    }
                }
            }
        }
    
    private:
        bool _alive;
        Ticker _aliveTicker;

        void _noMessageReceived() {
            // Probably dead... R.I.P.
            _alive = false;
        }
};

#endif // SOFTWARE_ALIVE_H