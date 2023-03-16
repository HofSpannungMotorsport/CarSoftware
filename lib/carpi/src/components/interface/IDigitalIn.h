#ifndef IDIGITALIN_H
#define IDIGITALIN_H

#include "ICommunication.h"


class IDigitalIn : public ICommunication {
    public:
        virtual int read() = 0;
        virtual void mode(PinMode pull) = 0;
};

#endif