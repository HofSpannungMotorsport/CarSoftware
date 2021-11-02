#ifndef SYNC_H
#define SYNC_H

// Include Debug-Info. Each define can be used on its own, or all by
//#define SYNC_DEBUG_ALL

// Errors should always be shown
#define SYNC_DEBUG_ERRORS

#ifdef SYNC_DEBUG_ALL
    #define SYNC_DEBUG_ERRORS
    #define SYNC_DEBUG_IN_BUFFER_DROPPED_MESSAGES
#endif //SYNC_DEBUG_ALL


#include "../componentIds.h"
#include "../deviceIds.h"
#include "../CarMessage.h"
#include "../../components/interface/IComponent.h"
#include "../IChannel.h"
#include "../../components/interface/ICommunication.h"
#include "../../runable/IRunable.h"
#include "../../HardConfig.h"
#include "../../helper/SharedTimer.h"

// To use the Stack-Vector, copy and uncomment the following lines to your ...Conf.h and set to the right values
//#define SYNC_USE_STACK_VECTOR
#ifdef SYNC_USE_STACK_VECTOR
//    #define SYNC_MAX_DEVICES_COUNT
//    #define SYNC_MAX_CHANNELS_COUNT
//    #define SYNC_MAX_COMPONENTS_COUNT
    #ifndef SYNC_MAX_DEVICES_COUNT
        #error "Max Devices Count not set!"
    #endif
    #ifndef SYNC_MAX_CHANNELS_COUNT
        #error "Max Channels Count not set!"
    #endif
    #ifndef SYNC_MAX_COMPONENTS_COUNT
        #error "Max Components Count not set!"
    #endif
#endif

#ifdef SYNC_USE_STACK_VECTOR
    #include "StackVector.h"
#endif

enum sync_message_command_t : uint8_t {
    SYNC_MESSAGE_COMMAND_CONFIRM = 0x0,
    SYNC_MESSAGE_COMMAND_CONFIRM_BROADCAST,
    SYNC_MESSAGE_COMMAND_LOST,
    SYNC_MESSAGE_COMMAND_LOST_BROADCAST,
    SYNC_MESSAGE_COMMAND_LOST_FIELD,
    SYNC_MESSAGE_COMMAND_LOST_FIELD_BROADCAST,
    SYNC_MESSAGE_COMMAND_LOST_PLACEHOLDER
};

typedef uint8_t messages_counter_t;


#include "SyncedOutBuffer.h"
#include "SyncedInBuffer.h"


class Sync : public IRunable {
    public:
        Sync(id_device_t thisId) : _thisId(thisId), _broadcastDummyChannel(*this) {
            _lastSentBroadcastOk.reset();
            _lastSentBroadcastOk.start();
        }

        #ifndef SYNC_USE_STACK_VECTOR
        Sync(id_device_t thisId, unsigned int reserveDevices, unsigned int reserveChannels, unsigned int reserveComponents) : Sync(thisId) {
            devices.reserve(reserveDevices);
            channels.reserve(reserveChannels);
            components.reserve(reserveComponents);
        }

        void finalize() {
            devices.shrink_to_fit();
            channels.shrink_to_fit();
            components.shrink_to_fit();
        }
        #endif

        void addDevice(IChannel &channel, id_device_t deviceId) {
            if (!_checkDeviceExist(deviceId)) {

                #if defined(VECTOR_EMPLACE_BACK_ENABLED) && !defined(SYNC_USE_STACK_VECTOR)
                    devices.emplace_back(channel, deviceId);
                #else
                    Device newDevice(channel, deviceId);
                    devices.push_back(newDevice);
                #endif

                _useChannel(channel);
            }
        }

        void addComponent(ICommunication &component, id_device_t deviceId) {
            if (!_checkComponentExist(component)) {
                #if defined(VECTOR_EMPLACE_BACK_ENABLED) && !defined(SYNC_USE_STACK_VECTOR)
                    components.emplace_back(component, deviceId);
                #else
                    Component newComponent(component, deviceId);
                    components.push_back(newComponent);
                #endif
            }
        }

        /**
         * @brief Give the Syncer a received message. The message is put in a buffer and processed in the next run(). Normally used by a channel.
         * 
         * @param carMessage Reference to the Message to be received
         */
        void receive(CarMessage &carMessage) {
            // Check if message is for this or all devices
            id_device_t receiverId = carMessage.getReceiverId();
            if (receiverId != _thisId && receiverId != DEVICE_ALL) return;

            // Search for sender device in device list
            for (Device &device : devices) {
                if (device.deviceId == carMessage.getSenderId()) {
                    // Add message in SyncedInBuffer OR (if an sync ok component) confirm / set lost messages in SyncedOutBuffer
                    if (carMessage.getComponentId() == componentId::getComponentId(COMPONENT_SYSTEM, COMPONENT_SYSTEM_SYNC_OK)) {
                        // -> OK Message to be delivered to SyncedOutBuffer
                        if (receiverId == _thisId) {
                            switch (carMessage.get(0)) {
                                case SYNC_MESSAGE_COMMAND_CONFIRM: {
                                        device.outBuffer.confirm(carMessage.get(1));
                                        device.resetOkTimer();
                                        break;
                                }

                                case SYNC_MESSAGE_COMMAND_LOST: {
                                    device.outBuffer.lost(carMessage.get(1));
                                    break;
                                }

                                case SYNC_MESSAGE_COMMAND_LOST_FIELD: {
                                    messages_counter_t to = carMessage.get(2);
                                    for (messages_counter_t count = carMessage.get(1); count != to; count++) {
                                        device.outBuffer.lost(count);
                                    }
                                    break;
                                }

                                case SYNC_MESSAGE_COMMAND_CONFIRM_BROADCAST: {
                                    device.lastConfirmedBroadcastId = carMessage.get(1);
                                    _updateConfirmedBroadcast();
                                    device.resetOkTimer();
                                    break;
                                }

                                case SYNC_MESSAGE_COMMAND_LOST_BROADCAST: {
                                    _outBroadcastBuffer.lost(carMessage.get(1));
                                    break;
                                }

                                case SYNC_MESSAGE_COMMAND_LOST_FIELD_BROADCAST: {
                                    messages_counter_t to = carMessage.get(2);
                                    for (messages_counter_t count = carMessage.get(1); count != to; count++) {
                                        _outBroadcastBuffer.lost(count);
                                    }
                                    break;
                                }

                                default: {
                                    #if defined(SYNC_DEBUG_ERRORS) && defined(MESSAGE_REPORT)
                                        printf("[Sync]@receive: Got COMPONENT_SYSTEM_SYNC_OK with implausible data at index 0 (?)\n");
                                    #endif
                                    break;
                                }
                            }
                        } else {
                            #if defined(SYNC_DEBUG_ERRORS) && defined(MESSAGE_REPORT)
                                printf("[Sync]@receive: Got COMPONENT_SYSTEM_SYNC_OK addressed to DEVICE_ALL (?)\n");
                            #endif
                        }
                    } else {
                        // -> Message for a real component to be delivered/added in SyncedInBuffer
                        CarMessage blankMessage;
                        blankMessage.setSenderId(_thisId);
                        blankMessage.setReceiverId(device.deviceId);
                        blankMessage.setPrioritise(true);

                        if (receiverId == DEVICE_ALL) {
                            if (device.inBroadcastBuffer.received(carMessage, *device.channel, blankMessage, true))
                                device.resetBroadcastSentIntervalTimer();
                        } else {
                            if (device.inBuffer.received(carMessage, *device.channel, blankMessage, false))
                                device.resetSentIntervalTimer();
                        }
                    }

                    break;
                }
            }
        }

        /**
         * @brief Send a CarMessage to be delivered automatically
         * 
         * @param carMessage reference to the message to be sent
         * @return true if the message is put into the buffer and may be sent soon
         * @return false if the outgoing buffer is full and the message WILL NOT be sent, or the component was not registered before
         */
        bool send(CarMessage &carMessage) {
            // Search for component in component list and get receiver device id
            bool idFound = false;
            id_device_t receiverDeviceId;
            for (Component &component : components) {
                if (component == carMessage.getComponentId()) {
                    receiverDeviceId = component.deviceId;
                    idFound = true;
                    break;
                }
            }

            // If ID found, search for Device to send the message to
            if (idFound) {
                carMessage.setSenderId(_thisId);
                carMessage.setReceiverId(receiverDeviceId);

                for (Device &device : devices) {
                    if (device.deviceId == receiverDeviceId) {
                        // Add Message in SyncedOutBuffer
                        return device.outBuffer.add(carMessage);
                    }
                }
            }

            return false;
        }

        /**
         * @brief Run the Syncer
         * 
         */
        void run() {
            // Go through all device buffers
            // Check all Buffers for pending messages
            for (Device &device : devices) {
                CarMessage carMessage;

                // First InBuffer for delivery
                while (device.inBuffer.deliver(carMessage)) {
                    for (Component &component : components) {
                        if (component == carMessage.getComponentId()) {
                            component.component->receive(carMessage);
                            break; // -> break component for loop
                        }
                    }
                }

                // Second InBroadcastBuffer for delivery
                while (device.inBroadcastBuffer.deliver(carMessage)) {
                    for (Component &component : components) {
                        if (component == carMessage.getComponentId()) {
                            component.component->receive(carMessage);
                            break; // -> break component for loop
                        }
                    }
                }

                // Then OutBuffer for pending messages to be sent
                while (device.outBuffer.sendNext(*device.channel));

                CarMessage blankMessage;
                blankMessage.setSenderId(_thisId);
                blankMessage.setReceiverId(device.deviceId);
                blankMessage.setPrioritise(true);

                // Last, check for interval message need
                if (device.intervalMessageNeeded()) {
                    if (device.inBuffer.sendIntervalMessage(*device.channel, blankMessage, false))
                        device.resetSentIntervalTimer();
                }

                if (device.intervalBroadcastMessageNeeded()) {
                    if (device.inBroadcastBuffer.sendIntervalMessage(*device.channel, blankMessage, true))
                        device.resetBroadcastSentIntervalTimer();
                }
            }


            // Same for the broadcast buffer
            while (_outBroadcastBuffer.sendNext(_broadcastDummyChannel));
        }

        bool messagesInOutBuffer() {
            if (_outBroadcastBuffer.pendingCount() > 0) return true;

            for (Device &device : devices)
                if (device.outBuffer.pendingCount() > 0)
                    return true;
            
            return false;
        }
    
    private:
        id_device_t _thisId;
        SyncedOutBuffer _outBroadcastBuffer;
        Timer _lastSentBroadcastOk;

        struct Device {
            /**
             * @brief Construct a new Device
             * 
             * @param _channel The Channel the device can be reached
             * @param _deviceId The ID to the Device for this route
             * @param _important If device is not reachable anymore, still try to send broadcast messages
             */
            #ifdef SYNC_USE_STACK_VECTOR
            Device() {}
            #endif
            Device(IChannel &_channel, id_device_t _deviceId)
            : channel(&_channel), deviceId(_deviceId) {
                resetOkTimer();
                resetSentIntervalTimer();
                resetBroadcastSentIntervalTimer();
            }

            IChannel *channel;
            id_device_t deviceId;

            // Data for Sending
            SharedTimer lastGotOk; // The time since the last got OK
            SharedTimer lastSentOk;
            SharedTimer lastSentBroadcastOk;
            messages_counter_t lastConfirmedBroadcastId = 0; // Number of the last confirmed Broadcast Message

            SyncedOutBuffer outBuffer;
            SyncedInBuffer inBuffer;
            SyncedInBuffer inBroadcastBuffer;

            bool operator==(id_device_t thatId) {
                if (deviceId == thatId)
                    return true;
                
                return false;
            }

            bool reachable() {
                return lastGotOk.read_ms() < STD_SYNC_INTERVAL_MESSAGE_TIMEOUT;
            }

            bool intervalMessageNeeded() {
                return lastSentOk.read_ms() >= STD_SYNC_INTERVAL_MESSAGE_TIME;
            }

            bool intervalBroadcastMessageNeeded() {
                return lastSentBroadcastOk.read_ms() >= STD_SYNC_INTERVAL_MESSAGE_TIME;
            }

            void resetOkTimer() {
                lastGotOk.reset();
                lastGotOk.start();
            }

            void resetSentIntervalTimer() {
                lastSentOk.reset();
                lastSentOk.start();
            }

            void resetBroadcastSentIntervalTimer() {
                lastSentBroadcastOk.reset();
                lastSentBroadcastOk.start();
            }
        };

        struct Channel {
            #ifdef SYNC_USE_STACK_VECTOR
            Channel() {}
            #endif
            Channel(IChannel &_channel) : channel(&_channel) {}

            IChannel *channel;

            bool operator==(IChannel &that) {
                if (channel == &that)
                    return true;
                
                return false;
            }
        };

        struct Component {
            #ifdef SYNC_USE_STACK_VECTOR
            Component() {}
            #endif
            Component(ICommunication &_component, id_device_t _deviceId)
            : component(&_component), deviceId(_deviceId) {}

            ICommunication *component;
            id_device_t deviceId;

            bool operator==(ICommunication &that) {
                if (&that == component)
                    return true;
                
                return false;
            }

            bool operator==(id_component_t thatId) {
                if (thatId == component->getComponentId())
                    return true;
                
                return false;
            }
        };

        class BroadcastDummyChannel : public IChannel {
            public:
                BroadcastDummyChannel(Sync& syncer) : _syncer(syncer) {}

                bool send(CarMessage &carMessage) {
                    return _syncer._broadcastSendCallback(carMessage);
                }

                void run() {
                    // no implementation needed
                }
            
            private:
                Sync &_syncer;
        };
        
        /**
         * @brief Local Broadcast Dummy Channel for broadcasting messages
         * 
         */
        BroadcastDummyChannel _broadcastDummyChannel;

        // Using Vector, but why? Its faster to iterate over a whole vector than using a slow map

        #ifdef SYNC_USE_STACK_VECTOR

        /**
         * @brief The Vector which will store all Devices (Device IDs and where to find them)
         * 
         */
        stack::vector<Device, SYNC_MAX_DEVICES_COUNT> devices;

        /**
         * @brief The Vector which will store all used channels, importent for broadcast messages
         * 
         */
        stack::vector<Channel, SYNC_MAX_CHANNELS_COUNT> channels;

        /**
         * @brief The Vector which will save the Components and where to find them
         * 
         */
        stack::vector<Component, SYNC_MAX_COMPONENTS_COUNT> components;

        #else

        /**
         * @brief The Vector which will store all Devices (Device IDs and where to find them)
         * 
         */
        std::vector<Device> devices;

        /**
         * @brief The Vector which will store all used channels, importent for broadcast messages
         * 
         */
        std::vector<Channel> channels;

        /**
         * @brief The Vector which will save the Components and where to find them
         * 
         */
        std::vector<Component> components;

        #endif


        // Helper

        /**
         * @brief Update the Confirmation(s) for the broadcast out buffer
         * 
         */
        inline void _updateConfirmedBroadcast() {
            messages_counter_t longestDistance = 0;

            if (_outBroadcastBuffer.pendingCount() == 0) {
                return;
            }

            for (Device &device : devices) {
                messages_counter_t thisDistance;

                if (_outBroadcastBuffer.idToIndexFromHead(device.lastConfirmedBroadcastId, thisDistance)) {
                    if (thisDistance > longestDistance)
                        longestDistance = thisDistance;
                }
            }

            _outBroadcastBuffer.confirm(_outBroadcastBuffer.getHeadMessageId() - longestDistance);
        }

        /**
         * @brief Reset and start a given timer
         * 
         * @param timer 
         */
        void _restartTimer(Timer &timer) {
            timer.reset();
            timer.start();
        }

        /**
         * @brief Flag a given Channel as beeing used for a device
         * 
         * @param channel 
         */
        void _useChannel(IChannel &channel) {
            if (!_checkChannelExist(channel)) {
                #if defined(VECTOR_EMPLACE_BACK_ENABLED) && !defined(SYNC_USE_STACK_VECTOR)
                    channels.emplace_back(channel);
                #else
                    Channel newChannel(channel);
                    channels.push_back(newChannel);
                #endif
            }
        }

        /**
         * @brief Callback for the Broadcast Channel Wrapper
         * 
         * @param carMessage 
         * @return true if the message was enqueued to all available channels
         * @return false if one or more channels refused to enqueue message
         */
        bool _broadcastSendCallback(CarMessage &carMessage) {
            bool sendFailure = false;

            for (Channel &channel : channels)
                if (!channel.channel->send(carMessage))
                    sendFailure = true;

            return !sendFailure;
        }


        // Check Exist Methods

        bool _checkDeviceExist(id_device_t deviceId) {
            for (Device &device : devices)
                if (device == deviceId)
                    return true;

            return false;
        }

        bool _checkChannelExist(IChannel &thatChannel) {
            for (Channel &channel : channels)
                if (channel == thatChannel)
                    return true;
            
            return false;
        }

        bool _checkComponentExist(ICommunication &thatComponent) {
            for (Component &component : components)
                if (component == thatComponent)
                    return true;
            
            return false;
        }
};

#endif // SSYNC_H