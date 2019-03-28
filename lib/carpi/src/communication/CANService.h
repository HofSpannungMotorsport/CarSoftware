#ifndef CANSERVICE_H
#define CANSERVICE_H

#ifndef DEVICE_CAN
    #define DEVICE_CAN // Otherwise a compiler issue (not error) -> defined by mbed before?
#endif

#include "platform/CircularBuffer.h"
#include <map>
#include <vector>
#include "can_ids.h"
#include "services/IService.h"
#include "components/interface/IID.h"
#include "components/handler/IMessageHandler.h"


#define STD_CAN_FREQUENCY 250000
#define TELEGRAM_IN_BUFFER_SIZE 64

#define CAN_TIMEOUT 0.100 // s

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

class CANService : public IService {
    public:
        CANService(PinName RX, PinName TX, int frequency = STD_CAN_FREQUENCY) : _can(RX, TX, frequency) {
            _can.attach(callback(this, &CANService::_messageReceived), CAN::RxIrq);
        }

        bool sendMessage(component_id_t id, can_object_type_t objectType) {
            component_exist_t componentExist = _registeredAddresses[id];

            #ifdef CAN_DEBUG
                pcSerial.printf("[CANService]@sendMessage: Try to send Message for ComponentID: 0x%x\tObjectType: 0x%x\n", id, objectType);
            #endif

            if (componentExist.exists) {
                CANMessage m = CANMessage();
                component_details_t component = _components[id];

                m.id = ID::getMessageId(component.priority, id, objectType);
                msg_build_result_t msgBuildResult = component.handler->buildMessage(component.component, m);

                #ifdef CAN_DEBUG
                    if (msgBuildResult == MSG_BUILD_OK) {
                        pcSerial.printf("[CANService]@sendMessage: Component 0x%x Message Build success\n", id);
                    } else {
                        pcSerial.printf("[CANService]@sendMessage: Component 0x%x Message Build error\n", id);
                    }
                #endif

                if (msgBuildResult == MSG_BUILD_ERROR) return false;

                int msgSendResult = 0;
                Timer timeout;
                timeout.start();
                while((msgSendResult != 1) && (timeout < CAN_TIMEOUT)) {
                    msgSendResult = _can.write(m);
                    wait(0.00000124);
                }

                #ifdef CAN_DEBUG
                    pcSerial.printf("[CANService]@sendMessage: Message for Component 0x%x with m.id 0x%x write result: %i (1 == Succes, 0 == Failed)\n", id, m.id, msgSendResult);
                #endif

                if (msgSendResult > 0) return true;
                else return false;
            } else { // else do nothing (component not registered before -> can't send messages for it == obvious)
                #ifdef CAN_DEBUG
                    pcSerial.printf("[CANService]@sendMessage: Can't send Message for Component 0x%x -> component not registered before (componentExist.exists == false)\n", id);
                #endif
            }

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

        bool processInbound() {
            bool success = true;
            #ifdef CAN_DEBUG
                pcSerial.printf("[CANService]@processInbound: Processing Inbound. Telegrams in Buffer: %i\n", _telegramsIn.size());
            #endif

            // Process received Telegrams saved in the _telegramsIn Buffer
            CANMessage m;
            while(_telegramsIn.pop(m)) {
                // search in _components for right component
                // call bridge with component pointer
                // check before if the component was registered before
                uint16_t id16 = (uint16_t)((m.id & 0x3FC) >> 2); // Filter out ComponentID and "convert" it to 8-Bit ID
                uint8_t id = id16;

                #ifdef CAN_DEBUG
                    pcSerial.printf("[CANService]@processInbound: Processing Message with ID: 0x%x\n", id);
                #endif

                component_exist_t componentExist = _registeredAddresses[id];
                if (componentExist.exists) {
                    component_details_t component = _components[id];
                    msg_parse_result_t msgParseResult = component.handler->parseMessage(component.component, m);

                    if (msgParseResult == MSG_PARSE_OK) {
                        #ifdef CAN_DEBUG
                            pcSerial.printf("[CANService]@processInbound: Message with ID 0x%x parsed successfully\n", id);
                        #endif
                    } else {
                        success = false;
                        #ifdef CAN_DEBUG
                            pcSerial.printf("[CANService]@processInbound: Message with ID 0x%x parsing failed!\n", id);
                        #endif
                    }

                } else { // else do nothing (because it could just be a message for a component on another device)
                    #ifdef CAN_DEBUG
                        pcSerial.printf("[CANService]@processInbound: Message not processed, component with ID 0x%x not registered before\n", id);
                    #endif
                }
            }

            return success;
        }

        void addComponent(component_id_t id, void* component, IMessageHandler<CANMessage>* handler, can_priority_t priority = NORMAL) {
            component_exist_t componentExist = _registeredAddresses[id];
            if (componentExist.exists) {
                #ifdef MESSAGE_REPORT
                    pcSerial.printf("[CANService]@addComponent: Component already added! ComponentID: 0x%x\n", id);
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
            _registeredAddresses[id] = newComponentExist;
            _components[id] = newComponent;

            #ifdef CAN_DEBUG
                pcSerial.printf("[CANService]@addComponent: Component added. ComponentID: 0x%x\n", id);
            #endif
        }

        void addComponent(void* component, IMessageHandler<CANMessage>* handler, can_priority_t priority = NORMAL) {
            addComponent(_calculateComponentId(component), component, handler, priority);
        }

        bool addComponentToSendLoop(component_id_t id) {
            component_exist_t componentExist = _registeredAddresses[id];
            if (componentExist.exists) {
                if (!componentExist.sendLoop) {
                    _registeredAddresses[id].sendLoop = true;
                    _sendLoopComponents.emplace_back(id);
                    #ifdef CAN_DEBUG
                        pcSerial.printf("[CANService]@addComponentToSendLoop: Component add to sendLoop. ComponentID: 0x%x\n", id);
                    #endif
                    return true;
                } else {
                    #ifdef MESSAGE_REPORT
                        pcSerial.printf("[CANService]@addComponentToSendLoop: Component already added to sendLoop! ComponentID: 0x%x\n", id);
                    #endif
                }
            } else {
                #ifdef MESSAGE_REPORT
                    pcSerial.printf("[CANService]@addComponentToSendLoop: Component not Registered before! ComponentID: 0x%x\n", id);
                #endif
            }

            return false;
        }

        bool addComponentToSendLoop(void* component) {
            return addComponentToSendLoop(_calculateComponentId(component));
        }

        bool processSendLoop() {
            bool success = true;

            #ifdef CAN_DEBUG
                pcSerial.printf("[CANService]@processSendLoop: Processing sendLoop. SendLoop size: %i\n", _sendLoopComponents.size());
            #endif

            for (auto componentId : _sendLoopComponents) {
                component_exist_t componentExist = _registeredAddresses[componentId];
                if (componentExist.sendLoop) {
                    if (sendMessage(componentId)) {
                        #ifdef CAN_DEBUG
                            pcSerial.printf("[CANService]@processSendLoop: Component in sendLoop processed successfully. ComponentID: 0x%x\n", componentId);
                        #endif
                    } else {
                        success = false;
                        #ifdef CAN_DEBUG
                            pcSerial.printf("[CANService]@processSendLoop: Component in sendLoop prcessing failed. ComponentID: 0x%x\n", componentId);
                        #endif
                    }
                } else {
                    success = false;
                    #ifdef CAN_DEBUG
                        pcSerial.printf("[CANService]@processSendLoop: Unable to process component in sendLoop (componentExist.sendLoop == false). ComponentID: 0x%x\n", componentId);
                    #endif
                }
            }

            return success;
        }

        virtual void run() {
            #ifdef CAN_DEBUG
                pcSerial.printf("[CANService]@run: Start CANService run()\n");
                if (processInbound())
                    pcSerial.printf("[CANService]@run: Success processInbound() (returned true)\n");
                else
                    pcSerial.printf("[CANService]@run: Failed processInbound() (returned false)\n");

                if (processSendLoop())
                    pcSerial.printf("[CANService]@run: Success processSendLoop() (returned true)\n");
                else
                    pcSerial.printf("[CANService]@run: Failed processSendLoop() (returned false)\n");
            #else
                processInbound();
                processSendLoop();
            #endif
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