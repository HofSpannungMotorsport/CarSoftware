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
#include "runable/IRunable.h"
#include "HardConfig.h"

//#define SYNC_DEBUG // for debugging

class Sync : public IRunable {
    public:
        Sync(id_device_t senderId) {
            _thisId = senderId;
        }

        bool addComponent(ICommunication &component, IChannel &channel, id_device_t receiverId) {
            // If component has already been added, deny adding it again
            if (_checkComponentExist(component)) {
                #if defined(SYNC_DEBUG) && defined(MESSAGE_REPORT)
                    pcSerial.printf("[Sync]@addComponent: Component with ID 0x%x already added!\n", component.getComponentId());
                #endif

                return false;
            }

            #ifdef USE_MBED
                // Construct new element at the end
                router.emplace_back(component, channel, receiverId);
            #else
                Route route(component, channel, receiverId);
                router.push_back(route);
            #endif // USE_MBED

            #if defined(SYNC_DEBUG) && defined(MESSAGE_REPORT)
                pcSerial.printf("[Sync]@addComponent: Component with ID 0x%x successfully added\n", component.getComponentId());
            #endif

            _addChannel(channel);

            return true;
        }

        bool addBridge(id_component_t componentId, IChannel &channelDevice1, IChannel &channelDevice2,
                                                   id_device_t deviceId1,    id_device_t deviceId2) {
            if (_checkBridgeExist(componentId)) {
                #if defined(SYNC_DEBUG) && defined(MESSAGE_REPORT)
                    pcSerial.printf("[Sync]@addBridge: Component with ID %i already added to Bridge!\n", componentId);
                #endif

                return false;
            }

            #ifdef USE_MBED
                bridger.emplace_back(componentId, channelDevice1, channelDevice2, deviceId1, deviceId2);
            #else
                Bridge bridge(componentId, channelDevice1, channelDevice2, deviceId1, deviceId2);
                bridger.push_back(bridge);
            #endif // USE_MBED

            #if defined(SYNC_DEBUG) && defined(MESSAGE_REPORT)
                pcSerial.printf("[Sync]@addBridge: Component with ID 0x%x successfully added to Bridge\n", componentId);
            #endif

            _addChannel(channelDevice1);
            _addChannel(channelDevice2);

            return true;
        }

        void receive(CarMessage &carMessage) {
            _incomingMessages.push(carMessage);
        }

        virtual void send(CarMessage &carMessage) {
            #if defined(SYNC_SENDING_DEBUG) && defined(MESSAGE_REPORT)
                pcSerial.printf("[Sync]@send: Try to send Message for component 0x%x\n", carMessage.getComponentId());
            #endif

            for (Route &route : router) {
                if (carMessage.getComponentId() == route.component->getComponentId()) {
                    carMessage.setSenderId(_thisId);
                    carMessage.setReceiverId(route.receiverId);
                    _send(carMessage, route.channel);

                    #if defined(SYNC_SENDING_DEBUG) && defined(MESSAGE_REPORT)
                        pcSerial.printf("[Sync]@send: Sent Message for component 0x%x to channel\n", carMessage.getComponentId());
                    #endif

                    break;
                }
            }
        }

        /*
            First send all messages sitting in the out queue,
            then compute received messages.
        */
        virtual void run() {
            for(Channel &channel : channels) {
                channel.channel->run();
            }

            CarMessage carMessage;
            while(_incomingMessages.pop(carMessage)) {
                _receive(carMessage);
            }
        }

        bool messageInQueue() {
            bool returnValue = false;
            for(Channel &channel : channels) {
                returnValue |= channel.channel->messageInQueue();
            }

            return returnValue;
        }

        void finalize() {
            router.shrink_to_fit();
            bridger.shrink_to_fit();
            channels.shrink_to_fit();
        }

    private:
        id_device_t _thisId;

        class Route {
            public:
                Route(ICommunication &_component, IChannel &_channel, id_device_t _receiverId)
                : component(&_component), channel(&_channel), receiverId(_receiverId) {}

                ICommunication *component;
                IChannel *channel;
                id_device_t receiverId;
        };

        class Bridge {
            public:
                Bridge(id_component_t _componentId, IChannel &_channelDevice1, IChannel &_channelDevice2,
                                                  id_device_t _deviceId1,    id_device_t _deviceId2)
                : componentId(_componentId), channelDevice1(&_channelDevice1), channelDevice2(&_channelDevice2),
                                         deviceId1(_deviceId1),            deviceId2(_deviceId2) {}

                id_component_t componentId;
                IChannel *channelDevice1;
                IChannel *channelDevice2;
                id_device_t deviceId1;
                id_device_t deviceId2;
        };

        class Channel {
            public:
                Channel(IChannel &_channel) : channel(&_channel) {}

                IChannel *channel;
        };

        // Using Vector, but why? Its faster to iterate over a whole vector than using a slow map

        // The Vector which will store all added components
        vector<Route> router;

        // The Vector which will store all devices that are bridged throu the current device
        vector<Bridge> bridger;

        // The Vector which will save all Channels only once for the run() method
        vector<Channel> channels;

        // The Vector which will temporarily store all incoming messages
        CircularBuffer<CarMessage, STD_SYNC_INCOMING_MESSAGES_BUFFER_SIZE, uint8_t> _incomingMessages;

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

        bool _checkChannelExist(IChannel &channel) {
            IChannel *channelToCheck = &channel;
            for(Channel &channel : channels) {
                if (channel.channel == channelToCheck) return true;
            }

            return false;
        }

        void _addChannel(IChannel &channel) {
            // At first, check if the Channel got regisered before
            if (_checkChannelExist(channel)) return;

            // If the channel did not got added before, add it now
            channels.emplace_back(channel);
        }

        void _receive(CarMessage &carMessage) {
            //#if defined(SYNC_DEBUG) && defined(MESSAGE_REPORT)
                printf("[Sync]@receive: Received Message for component 0x%x\n", carMessage.getComponentId());
            //#endif

            // Check if the message is assigned to this device
            if (carMessage.getReceiverId() == _thisId) {
                // -> The Message is for this device
                for(Route &route : router) {
                    if (route.component->getComponentId() == carMessage.getComponentId()) {
                        route.component->receive(carMessage);

                        //#if defined(SYNC_DEBUG) && defined(MESSAGE_REPORT)
                            printf("[Sync]@receive: Pushed message to Component 0x%x\n", carMessage.getComponentId());
                        //#endif

                        break;
                    }
                }
            } else {
                // -> The Message is for a different device
                // -> Maybe it could be bridged over this device?
                for(Bridge &bridge : bridger) {
                    if (carMessage.getComponentId() == bridge.componentId) {
                        #if defined(SYNC_DEBUG) && defined(MESSAGE_REPORT)
                            bool bridged = false;
                        #endif

                        if (bridge.deviceId1 == carMessage.getSenderId() || bridge.deviceId1 == DEVICE_ALL) {
                            if (bridge.deviceId2 == carMessage.getReceiverId() || carMessage.getReceiverId() == DEVICE_ALL) {
                                _send(carMessage, bridge.channelDevice2);
                                #if defined(SYNC_DEBUG) && defined(MESSAGE_REPORT)
                                    bridged = true;
                                #endif
                            }
                        } else if (bridge.deviceId2 == carMessage.getSenderId() || bridge.deviceId2 == DEVICE_ALL) {
                            if (bridge.deviceId1 == carMessage.getReceiverId() || carMessage.getReceiverId() == DEVICE_ALL) {
                                _send(carMessage, bridge.channelDevice1);
                                #if defined(SYNC_DEBUG) && defined(MESSAGE_REPORT)
                                    bridged = true;
                                #endif
                            }
                        }

                        #if defined(SYNC_DEBUG) && defined(MESSAGE_REPORT)
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
};

#endif // SSYNC_H