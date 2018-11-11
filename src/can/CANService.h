#ifndef CANSERVICE_H
#define CANSERVICE_H

#ifndef DEVICE_CAN
    #define DEVICE_CAN // Otherwise a compiler issue (not error) -> defined by mbed before?
#endif

#include "mbed.h"
#include <CircularBuffer.h>
#include "IMessageHandler.h"
#include <map>
#include <vector>
#include "can_ids.h"
#include "../components/interface/IID.h"

#define TELEGRAM_IN_BUFFER_SIZE 64

struct component_details_t {
    IMessageHandler<CANMessage>* handler;
    void* component;
    uint8_t objectType;
    can_priority_t priority;
};

struct component_exist_t {
    bool exists = false,
         sendLoop = false;
};

class CANService {
    public:
        CANService(PinName RX, PinName TX) : _can(RX, TX) {
            _can.attach(callback(this, &CANService::_messageReceived), CAN::RxIrq);
        }

        CANService(PinName RX, PinName TX, int frequency) : _can(RX, TX, frequency) {
            _can.attach(callback(this, &CANService::_messageReceived), CAN::RxIrq);
        }

        bool sendMessage(component_id_t id, can_object_type_t objectType) {
            component_exist_t componentExist = _registeredAddresses[id];
            if (componentExist.exists) {
                CANMessage m = CANMessage();
                component_details_t component = _components[id];

                m.id = ID::getMessageId(component.priority, id, objectType);

                if (component.handler->buildMessage(component.component, m)) return false;

                if (_can.write(m) > 0) return true;
                else return false;
            } // else do nothing (component not registered before -> can't send messages for it == obvious)

            return false;
        }

        bool sendMessage(component_id_t id) {
            component_details_t component = _components[id];
            can_object_type_t objectType = _getComponentObjectType(component.component);
            return sendMessage(id, objectType);
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
                // check before if the component was registered before
                uint8_t id = (uint8_t)((m.id & 0x3FC) >> 2); // Filter out ComponentID and "convert" it to 8-Bit ID

                component_exist_t componentExist = _registeredAddresses[id];
                if (componentExist.exists) {
                    component_details_t component = _components[id];
                    component.handler->parseMessage(component.component, m);
                } // else do nothing (because it could just be a message for a component on another device)
            }
        }

        void addComponent(component_id_t id, void* component, IMessageHandler<CANMessage>* handler, can_priority_t priority = NORMAL) {
            component_exist_t componentExist = _registeredAddresses[id];
            if (componentExist.exists) {
                #ifdef MESSAGE_REPORT
                    pcSerial.printf("[CANService]@addComponent: Component already added! ComponentID: %i", id);
                #endif
            }

            // Create new component to process Messages for it
            component_details_t newComponent;
            newComponent.handler = handler;
            newComponent.component = component;
            newComponent.priority = priority;

            // To be able to check later if the component has been registered before, save it in a different map to check
            component_exist_t newComponentExist;
            newComponentExist.exists = true;

            // Using emplace if a component gets registered twice on the same ID -> overwrite it
            // (otherwise it would be added at the next free key -> not good...)
            _registeredAddresses.emplace(id, newComponentExist);
            _components.emplace(id, newComponent);
        }

        void addComponent(void* component, IMessageHandler<CANMessage>* handler, can_priority_t priority = NORMAL) {
            addComponent(_calculateComponentId(component), component, handler, priority);
        }

        bool addComponentToSendLoop(component_id_t id) {
            component_exist_t componentExist = _registeredAddresses[id];
            if (componentExist.exists) {
                if (componentExist.sendLoop) {
                    _registeredAddresses[id].sendLoop = true;
                    _sendLoopComponents.emplace_back(id);
                    return true;
                } else {
                    #ifdef MESSAGE_REPORT
                        pcSerial.printf("[CANService]@addComponentToSendLoop: Component already added to sendLoop! ComponentID: %i", id);
                    #endif
                }
            } else {
                #ifdef MESSAGE_REPORT
                    pcSerial.printf("[CANService]@addComponentToSendLoop: Component not Registered before! ComponentID: %i", id);
                #endif
            }

            return false;
        }

        bool addComponentToSendLoop(void* component) {
            return addComponentToSendLoop(_calculateComponentId(component));
        }

        bool processSendLoop() {
            bool success = true;
            for (auto componentId : _sendLoopComponents) {
                component_exist_t componentExist = _registeredAddresses[componentId];
                if (componentExist.sendLoop) {
                    sendMessage(componentId);
                } else {
                    success = false;
                }
            }

            return success;
        }

        uint8_t run() {
            bool sendSuccess = processSendLoop();

            if (sendSuccess) {
                return 0;
            } else {
                return 1;
            }
        }

    private:
        CAN _can;
        CircularBuffer<CANMessage, TELEGRAM_IN_BUFFER_SIZE, uint8_t> _telegramsIn;

        // Saves all registered Components with its Message Handler
        std::map<component_id_t, component_details_t> _components;
        /*
            Because of the struct defined at the beginning of the file, you
            can store if a component has been registered in the following map.

            If it wasn't registered, the map will return the datatype as it has
            just been constructed -> component.exists == false (as in the struct)...
        */
        std::map<component_id_t, component_exist_t> _registeredAddresses;

        // Saves all (already registered) components to be send repeatedly
        vector<component_id_t> _sendLoopComponents;

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

        can_object_type_t _getComponentObjectType(void* component) {
            IID *componentObjectType = (IID*)component;
            can_object_type_t objectType = componentObjectType->getObjectType();
            return objectType;
        }
};

#endif // CANSERVICE_H