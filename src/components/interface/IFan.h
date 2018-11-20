#ifndef IFAN_H
#define IFAN_H

#include "IID.h"

typedef float fan_speed_t;

class IFan : public IID {
    public:
        virtual void setSpeed(fan_speed_t speed) = 0;
        virtual fan_speed_t getSpeed() = 0;
};

#endif // IFAN_H