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

        alive_status_t getStatus() {
            if (getAlive()) {
                return 0;
            } else {
                alive_error_type_t error = ALIVE_ERROR_DEAD;
                return error;
            }
        }
    
    private:
        bool _alive;
};

#endif // SOFTWARE_ALIVE_H