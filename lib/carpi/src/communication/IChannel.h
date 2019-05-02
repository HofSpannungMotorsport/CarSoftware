#ifndef ICHANNEL_H
#define ICHANNEL_H

#include "communication/CarMessage.h"

class IChannel {
    public:
        virtual void send(CarMessage &carMessage) = 0;
};

#endif // ICHANNEL_H