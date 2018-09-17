#ifndef CANSERVICE_H
#define CANSERVICE_H

#define DEVICE_CAN

#include "mbed.h"
#include <CircularBuffer.h>
#include "IMessageHandler.h"
#include <map>
#include "can_ids.h"


struct component_details_t
{
    IMessageHandler<CANMessage>* handler;
    void* component;
    can_prio_t priority;
};

class CANService
{
    private:
        CAN* _can;
        CircularBuffer<CANMessage,100,uint16_t> _telegramsIn;
        std::map<uint8_t,component_details_t> _components;

        void msgReceived()
        {
            CANMessage *m = new CANMessage();
            while(_can->read(*m))
            {
                _telegramsIn.push(*m);
            }

        }
    public:
        CANService()
        {
            _can->attach(this,&msgReceived,CAN::RxIrq);
        }
        void processInbound()
        {
            CANMessage m;
            while(_telegramsIn.pop(m))
            {
                // search in _components for right component
                // call bridge with component pointer
            }
        }
        void addComponent(uint8_t id, void* component, IMessageHandler<CANMessage>* handler, can_prio_t priority = NORMAL)
        {
            // add to _components
        }
};

#endif