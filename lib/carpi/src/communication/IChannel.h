#ifndef ICHANNEL_H
#define ICHANNEL_H

#include "communication/CarMessage.h"
#include "runable/IRunable.h"

class IChannel : public IRunable {
    public:
        virtual void send(CarMessage &carMessage) = 0;
        virtual bool messageInQueue() = 0;
};

#endif // ICHANNEL_H