#ifndef IPUMP_H
#define IPUMP_H

#include "IID.h"

typedef float pump_speed_t;

class IPump : public IID {
    public:
        virtual void setSpeed(pump_speed_t speed) = 0;
        virtual pump_speed_t getSpeed() = 0;
};

#endif // IPUMP_H