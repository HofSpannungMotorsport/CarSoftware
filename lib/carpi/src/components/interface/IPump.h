#ifndef IPUMP_H
#define IPUMP_H

#include "IComponent.h"

typedef float pump_speed_t;

class IPump : public IComponent {
    public:
        virtual void setSpeed(pump_speed_t speed) = 0;
        virtual pump_speed_t getSpeed() = 0;

        virtual void setEnable(bool enable) = 0;
        virtual bool getEnable() = 0;
};

#endif // IPUMP_H