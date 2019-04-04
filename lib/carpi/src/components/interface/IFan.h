#ifndef IFAN_H
#define IFAN_H

#include "IComponent.h"

enum fan_state_t : bool {
    FAN_OFF = false,
    FAN_ON  = true
};

class IFan : public IComponent {
    public:
        virtual void setState(fan_state_t state) = 0;
        virtual fan_state_t getState() = 0;
};

#endif // IFAN_H