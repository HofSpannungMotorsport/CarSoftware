#ifndef SYNTETIC_SYNC_H
#define SYNTETIC_SYNC_H

#include "carpi.h"

const id_sub_component_t COMPONENT_SUB_ID_1 = COMPONENT_ALIVE_MASTER;
const id_sub_component_t COMPONENT_SUB_ID_2 = COMPONENT_ALIVE_PEDAL;

class SynteticSync : public Sync {
    public:
        SynteticSync(id_device_t senderId) : Sync(senderId) {}

        void addComponent1(ICommunication &component) {
            component1 = &component;
        }

        void addComponent2(ICommunication &component) {
            component2 = &component;
        }

        virtual void send(CarMessage &carMessage) {
            _messageBuffer.push(carMessage);
        }

        virtual void run() {
            CarMessage carMessage;

            while(_messageBuffer.pop(carMessage)) {
                if ((carMessage.getComponentId() & 0xF) == COMPONENT_SUB_ID_1) {
                    carMessage.setComponentId((carMessage.getComponentId() & 0xF0) | COMPONENT_SUB_ID_2);
                    component2->receive(carMessage);
                }

                if ((carMessage.getComponentId() & 0xF) == COMPONENT_SUB_ID_2) {
                    carMessage.setComponentId((carMessage.getComponentId() & 0xF0) | COMPONENT_SUB_ID_1);
                    component1->receive(carMessage);
                }
            }
        }
    
    protected:
        ICommunication* component1;
        ICommunication* component2;
        CircularBuffer<CarMessage, SYNC_INCOMING_MESSAGES_BUFFER_SIZE, uint8_t> _messageBuffer;
};

#endif // SNYTETIC_SYNC_H