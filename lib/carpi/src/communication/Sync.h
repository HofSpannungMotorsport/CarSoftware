#ifndef SYNC_H
#define SYNC_H

#include <vector>

using namespace std;

#include "componentIds.h"
#include "deviceIds.h"
#include "CarMessage.h"
#include "IComponent.h"
#include "IChannel.h"
#include "components/interface/ICommunication.h"

#define SYNC_DEBUG // for debugging

class Sync {
    public:
        Sync(id_device_t senderId) {
            _thisId = senderId;
        }

        bool addComponent(ICommunication &component, IChannel &channel, id_device_t receiverId) {
            // If component has already been added, deny adding it again
            if (_checkComponentExist(component)) {
                #ifdef SYNC_DEBUG
                    pcSerial.printf("[Sync]@addComponent: Component with ID 0x%x already added!\n", component.getComponentId());
                #endif

                return false;
            }

            // Construct new element at the end
            router.emplace_back(component, channel, receiverId);

            #ifdef SYNC_DEBUG
                pcSerial.printf("[Sync]@addComponent: Component with ID 0x%x successfully added\n", component.getComponentId());
            #endif

            return true;
        }

        bool addBridge(id_component_t componentId, IChannel &channelDevice1, IChannel &channelDevice2,
                                                   id_device_t deviceId1,    id_device_t deviceId2) {
            if (_checkBridgeExist(componentId)) {
                #ifdef SYNC_DEBUG
                    pcSerial.printf("[Sync]@addBridge: Component with ID %i already added to Bridge!\n", componentId);
                #endif

                return false;
            }

            bridger.emplace_back(componentId, channelDevice1, channelDevice2, deviceId1, deviceId2);

            #ifdef SYNC_DEBUG
                pcSerial.printf("[Sync]@addBridge: Component with ID 0x%x successfully added to Bridge\n", componentId);
            #endif

            return true;
        }

        void receive(CarMessage &carMessage) {
            #ifdef SYNC_DEBUG
                pcSerial.printf("[Sync]@receive: Received Message for component 0x%x\n", carMessage.getComponentId());
            #endif

            // Check if the message is assigned to this device
            if (carMessage.getReceiverId() == _thisId) {
                // -> The Message is for this device
                for(Route &route : router) {
                    if (route.component->getComponentId() == carMessage.getComponentId()) {
                        route.component->receive(carMessage);

                        #ifdef SYNC_DEBUG
                            pcSerial.printf("[Sync]@receive: Pushed message to Component 0x%x\n", carMessage.getComponentId());
                        #endif

                        break;
                    }
                }
            } else {
                // -> The Message is for a different device
                // -> Maybe it could be bridged over this device?
                for(Bridge &bridge : bridger) {
                    if (carMessage.getComponentId() == bridge.componentId) {
                        #ifdef SYNC_DEBUG
                            bool bridged = false;
                        #endif

                        if (bridge.deviceId1 == carMessage.getSenderId()) {
                            if (bridge.deviceId2 == carMessage.getReceiverId()) {
                                _send(carMessage, bridge.channelDevice2);
                                #ifdef SYNC_DEBUG
                                    bridged = true;
                                #endif
                            }
                        } else if (bridge.deviceId2 == carMessage.getSenderId()) {
                            if (bridge.deviceId1 == carMessage.getReceiverId()) {
                                _send(carMessage, bridge.channelDevice1);
                                #ifdef SYNC_DEBUG
                                    bridged = true;
                                #endif
                            }
                        }

                        #ifdef SYNC_DEBUG
                            if(bridged) {
                                pcSerial.printf("[Sync]@receive: Succesfully bridged message for component 0x%x\n", carMessage.getComponentId());
                            } else {
                                pcSerial.printf("[Sync]@receive: Error bridging message for component 0x%x\n", carMessage.getComponentId());
                            }
                        #endif

                        break;
                    }
                }
            }
        }

        void send(CarMessage &carMessage) {
            #ifdef SYNC_DEBUG
                pcSerial.printf("[Sync]@send: Try to send Message for component 0x%x\n", carMessage.getComponentId());
            #endif

            for (Route &route : router) {
                if (carMessage.getComponentId() == route.component->getComponentId()) {
                    carMessage.setSenderId(_thisId);
                    carMessage.setReceiverId(route.receiverId);
                    _send(carMessage, route.channel);

                    #ifdef SYNC_DEBUG
                        pcSerial.printf("[Sync]@send: Sent Message for component 0x%x to channel\n", carMessage.getComponentId());
                    #endif

                    break;
                }
            }
        }

    private:
        id_device_t _thisId;

        class Route {
            public:
                Route(ICommunication &component, IChannel &channel, id_device_t receiverId)
                : component(&component), channel(&channel), receiverId(receiverId) {}

                ICommunication *component;
                IChannel *channel;
                id_device_t receiverId;
        };

        class Bridge {
            public:
                Bridge(id_component_t componentId, IChannel &channelDevice1, IChannel &channelDevice2,
                                                  id_device_t deviceId1,    id_device_t deviceId2)
                : componentId(componentId), channelDevice1(&channelDevice1), channelDevice2(&channelDevice2),
                                         deviceId1(deviceId1),            deviceId2(deviceId2) {}

                id_component_t componentId;
                IChannel *channelDevice1;
                IChannel *channelDevice2;
                id_device_t deviceId1;
                id_device_t deviceId2;
        };

        // Using Vector, but why? Its faster to iterate over a whole vector than using a slow map

        // The Vector which will store all added components
        vector<Route> router;

        // The Vector which will store all devices that are bridged throu the current device
        vector<Bridge> bridger;

        bool _checkComponentExist(id_component_t componentId) {
            for(Route &route : router) {
                if (route.component->getComponentId() == componentId) {
                    return true;
                }
            }

            return false;
        }

        bool _checkComponentExist(ICommunication &component) {
            return _checkComponentExist(component.getComponentId());
        }

        bool _checkBridgeExist(id_component_t componentId) {
            for(Bridge &bridge : bridger) {
                if (bridge.componentId == componentId) {
                    return true;
                }
            }

            return false;
        }

        void _send(CarMessage &carMessage, IChannel *channel) {
            channel->send(carMessage);
        }
};

#endif // SSYNC_H