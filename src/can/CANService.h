#ifndef CANSERVICE_H
#define CANSERVICE_H

#define DEVICE_CAN

#include "mbed.h"
#include <CircularBuffer.h>
#include "IMessageHandler.h"
#include <map>
#include "can_ids.h"
#include "../components/interface/IID.h"


struct component_details_t {
    IMessageHandler<CANMessage>* handler;
    void* component;
    can_prio_t priority;
};

class CANService {
    public:
        CANService(PinName RX, PinName TX) : _can(RX, TX) {
            _can.attach(this, &_messageReceived, CAN::RxIrq);
        }

        CANService(PinName RX, PinName TX, int frequency) : _can(RX, TX, frequency) {
            _can.attach(this, &_messageReceived, CAN::RxIrq);
        }

        bool sendMessage(component_id_t id) {
            CANMessage m = CANMessage;
            component_details_t component = _components[id];

            if (component.handler->buildMessage(m))
                return false;

            if (_can.write(m) > 0) return true;
        }

        bool sendMessage(void* component) {
            return sendMessage(_calculateComponentId(component));
        }

        void processInbound() {
            // Process received Telegrams saved in the _telegramsIn Buffer
            CANMessage m;
            while(_telegramsIn.pop(m)) {
                // search in _components for right component
                // call bridge with component pointer

                component_details_t component = _components[(uint8_t)((m.id & 0x3FC) >> 2)]; // Filter out ComponentID and "convert" it to 8-Bit ID
                component.handler->parseMessage(component.component, m);
            }
        }

        void addComponent(component_id_t id, void* component, IMessageHandler<CANMessage>* handler, can_prio_t priority = NORMAL) {
            // Create new component to process Messages for it
            component_details_t newComponent;
            newComponent.handler = handler;
            newComponent.component = component;
            newComponent.priority = priority;

            _components.emplace(id, newComponent);
        }

        void addComponent(void* component, IMessageHandler<CANMessage>* handler, can_prio_t priority = NORMAL) {
            addComponent(_calculateComponentId(component), component, handler, priority);
        }
        
    private:
        CAN _can;
        CircularBuffer<CANMessage, 100, uint8_t> _telegramsIn;
        std::map<component_id_t, component_details_t> _components;

        void _messageReceived() {
            // Put received Message in a Buffer to process later
            CANMessage m = CANMessage();
            while(_can.read(m)) {
                _telegramsIn.push(m);
            }
        }

        component_id_t _calculateComponentId(void* component) {
            IID *componentId = (IID*)component;
            component_id_t id = ID::getComponentId(componentId->getTelegramTypeId(), componentId->getComponentId());
            return id;
        }
};

#endif