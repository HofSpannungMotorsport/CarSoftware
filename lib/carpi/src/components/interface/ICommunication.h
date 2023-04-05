#ifndef ICOMMUNICATION_H
#define ICOMMUNICATION_H

#include "IComponent.h"
#include "../../communication/CarMessage.h"

class ICommunication : public IComponent {
    public:
        virtual void receive(CarMessage &carMessage) = 0;
        virtual void resetCallback(id_device_t deviceId) = 0;
};

#endif // ICOMMUNICATION_H