#ifndef IPUMP_H
#define IPUMP_H

#include "IComponent.h"

typedef float pump_speed_t;
typedef int pump_enable_t;

class IPump : public IComponent {
    public:
        virtual void setSpeed(pump_speed_t speed) = 0;
        virtual pump_speed_t getSpeed() = 0;

        virtual void setEnable(pump_enable_t enable) = 0;
        virtual pump_enable_t getEnable() = 0;
};

#endif // IPUMP_H