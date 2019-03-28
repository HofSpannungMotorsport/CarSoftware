#ifndef IFAN_H
#define IFAN_H

#include "IID.h"

enum fan_state_t : bool {
    FAN_OFF = false,
    FAN_ON  = true
};

class IFan : public IID {
    public:
        virtual void setState(fan_state_t state) = 0;
        virtual fan_state_t getState() = 0;
};

#endif // IFAN_H