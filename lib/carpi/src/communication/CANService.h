#ifndef CANSERVICE_H
#define CANSERVICE_H

#ifndef DEVICE_CAN
    #define DEVICE_CAN // Otherwise a compiler issue (not error) -> defined by mbed before?
#endif

#include "platform/CircularBuffer.h"
#include <map>
#include <vector>
#include "services/IService.h"
#include "components/interface/ICommunication.h"
#include "CarMessage.h"


#define STD_CAN_FREQUENCY 100000
#define TELEGRAM_IN_BUFFER_SIZE 64

#define CAN_TIMEOUT 0.100 // s

struct component_details_t {
    ICommunication *component;
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

        bool sendMessage(id_component_t id, id_device_t senderId, id_device_t receiverId) {
            component_exist_t componentExist = _registeredAddresses[id];

            #ifdef CAN_DEBUG
                pcSerial.printf("[CANService]@sendMessage: Try to send Message for ComponentID: 0x%x\n", id);
            #endif

            if (componentExist.exists) {
                CarMessage m;
                component_details_t component = _components[id];

                m.setSenderId(senderId);
                m.setReceiverId(receiverId);
                message_build_result_t msgBuildResult = component.component->buildMessage(m);

                #ifdef CAN_DEBUG
                    if (msgBuildResult == MESSAGE_BUILD_OK) {
                        pcSerial.printf("[CANService]@sendMessage: Component 0x%x Message Build success\n", id);
                    } else {
                        pcSerial.printf("[CANService]@sendMessage: Component 0x%x Message Build error\n", id);
                    }
                #endif

                if (msgBuildResult == MESSAGE_BUILD_ERROR) return false;


                // Convert CarMessage to CANMessage and send all sub-messages
                int msgSendResult = 0;
                for (car_sub_message_t &subMessage : m.subMessages) {
                    msgSendResult = 0;

                    CANMessage canMessage = CANMessage();
                    canMessage.format = CANStandard;
                    canMessage.id = deviceId::getMessageHeader(m.getSenderId(), m.getReceiverId());
                    canMessage.len = subMessage.length + 1;
                    canMessage.data[0] = id;
                    
                    for (int i = 0; i < subMessage.length; i++) {
                        canMessage.data[i+1] = subMessage.data[i];
                    }

                    Timer timeout;
                    timeout.start();
                    while((msgSendResult != 1) && (timeout < CAN_TIMEOUT)) {
                        msgSendResult = _can.write(canMessage);
                        wait(0.00000124);
                    }
                }

                #ifdef CAN_DEBUG
                    pcSerial.printf("[CANService]@sendMessage: Message for Component 0x%x write result: %i (1 == Succes, 0 == Failed)\n", id, msgSendResult);
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

        bool sendMessage(ICommunication* component, id_device_t receiverId) {
            return sendMessage(component->getComponentId(), _deviceId, receiverId);
        }

        bool broadcastMessage(id_component_t id) {
            return sendMessage(id, _deviceId, DEVICE_ALL);
        }

        bool broadcastMessage(ICommunication* component) {
            return broadcastMessage(component->getComponentId());
        }

        bool processInbound() {
            bool success = true;
            #ifdef CAN_DEBUG
                pcSerial.printf("[CANService]@processInbound: Processing Inbound. Telegrams in Buffer: %i\n", _telegramsIn.size());
            #endif

            // Process received Telegrams saved in the _telegramsIn Buffer
            CANMessage m;
            while(_telegramsIn.pop(m)) {
                // Search in _components for right component
                // Check before if the component was registered before
                // So read the first byte of the message which represents the component id
                id_component_t id = m.data[0];

                #ifdef CAN_DEBUG
                    pcSerial.printf("[CANService]@processInbound: Processing Message with ID: 0x%x\n", id);
                #endif

                component_exist_t componentExist = _registeredAddresses[id];
                if (componentExist.exists) {
                    // Convert CANMessage to CarMessage
                    CarMessage carMessage;

                    car_sub_message_t subMessage;
                    subMessage.length = m.len - 1;
                    
                    for(int i = 0; i < subMessage.length; i++) {
                        subMessage.data[i] = m.data[i+1];
                    }

                    carMessage.addSubMessage(subMessage);

                    component_details_t component = _components[id];
                    message_parse_result_t msgParseResult = component.component->parseMessage(carMessage);

                    if (msgParseResult == MESSAGE_PARSE_OK) {
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

        void addComponent(ICommunication* component) {
            id_component_t id = component->getComponentId();
            component_exist_t componentExist = _registeredAddresses[id];
            if (componentExist.exists) {
                #ifdef MESSAGE_REPORT
                    pcSerial.printf("[CANService]@addComponent: Component already added! ComponentID: 0x%x\n", id);
                #endif
            }

            // Create new component to process Messages for it
            component_details_t newComponent;
            newComponent.component = component;

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

        bool addComponentToSendLoop(ICommunication *component) {
            id_component_t id = component->getComponentId();
            component_exist_t componentExist = _registeredAddresses[id];
            if (componentExist.exists) {
                if (!componentExist.sendLoop) {
                    _registeredAddresses[id].sendLoop = true;
                    _sendLoopComponents.push_back(id);
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

        bool processSendLoop() {
            bool success = true;

            #ifdef CAN_DEBUG
                pcSerial.printf("[CANService]@processSendLoop: Processing sendLoop. SendLoop size: %i\n", _sendLoopComponents.size());
            #endif

            for (auto componentId : _sendLoopComponents) {
                component_exist_t componentExist = _registeredAddresses[componentId];
                if (componentExist.sendLoop) {
                    if (broadcastMessage(componentId)) {
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

        void setSenderId(id_device_t senderId) {
            _deviceId = senderId;
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

            uint8_t tdError = _can.tderror();
            uint8_t rdError = _can.rderror();
            if (tdError > 0 || rdError > 0) {
                #ifdef REPORT_CAN_ERROR
                pcSerial.printf("[CANService]@run: Detected CAN Error: td: %i\t rd: %i\n", tdError, rdError);
                #endif

                // Reset to recover from passive mode
                if (tdError >= 127 || rdError >= 127) {
                    //_can.reset(); -> Removed as useless
                }
            }
        }

    private:
        CAN _can;
        CircularBuffer<CANMessage, TELEGRAM_IN_BUFFER_SIZE, uint8_t> _telegramsIn;

        // Saves all registered Components with its Message Handler
        map<id_component_t, component_details_t> _components;
        /*
            Because of the struct defined at the beginning of the file, you
            can store if a component has been registered in the following map.

            If it wasn't registered, the map will return the datatype as it has
            just been constructed -> component.exists == false (as in the struct)...
        */
        map<id_component_t, component_exist_t> _registeredAddresses;

        // Saves all (already registered) components to be send repeatedly
        vector<id_component_t> _sendLoopComponents;

        // The address for this device
        id_device_t _deviceId = DEVICE_ALL;

        void _messageReceived() {
            // Put received Message in a Buffer to process later
            CANMessage m = CANMessage();
            bool second = false;

            while(_can.read(m)) {
                _telegramsIn.push(m);

                // Wait for preemption if multiple messages are received at once
                if (!second) {
                    second = true;
                } else {
                    wait(0.000001);
                }
            }
        }
};

#endif // CANSERVICE_H