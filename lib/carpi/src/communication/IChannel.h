#ifndef ICHANNEL_H
#define ICHANNEL_H

#include "CarMessage.h"
#include "../runable/IRunable.h"

class IChannel : public IRunable {
    public:
        virtual bool send(CarMessage &carMessage) = 0;
};

#endif // ICHANNEL_H