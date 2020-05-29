#ifndef SYNC_H
#define SYNC_H

// Include Debug-Info. Each define can be used on its own, or all by
//#define SYNC_DEBUG_ALL

// Errors should always be shown
#define SYNC_DEBUG_ERRORS

#ifdef SYNC_DEBUG_ALL
    #define SYNC_DEBUG_ERRORS
#endif //SYNC_DEBUG_ALL


#include "componentIds.h"
#include "deviceIds.h"
#include "CarMessage.h"
#include "../components/interface/IComponent.h"
#include "IChannel.h"
#include "../components/interface/ICommunication.h"
#include "../runable/IRunable.h"
#include "../HardConfig.h"
#include "../helper/SharedTimer.h"

// To use the Stack-Vector, copy and uncomment the following lines to your ...Conf.h and set to the right values
//#define SYNC_USE_STACK_VECTOR
//#define SYNC_MAX_DEVICES_COUNT
//#define SYNC_MAX_CHANNELS_COUNT
//#define SYNC_MAX_COMPONENTS_COUNT

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

/**
 * @brief Essentially a Ring-Buffer with some extra stuff for the syncing
 * 
 */
class SyncedBuffer {
    public:
        /**
         * @brief Check if the Message Buffer is Empty
         * 
         * @return true 
         * @return false 
         */
        inline bool empty() {
            return head == tail && !_full;
        }

        /**
         * @brief Get the amount of Messages in the Buffer
         * 
         * @return messages_counter_t 
         */
        messages_counter_t getMessageCount() {
            if (head > tail) {
                return (head - tail);
            } else if (head < tail) {
                messages_counter_t messageCount = (STD_SYNC_MESSAGES_BUFFER_SIZE - 1) - tail;
                messageCount += (head + 1);
                
                return messageCount;
            } else if (_full) {
                return STD_SYNC_MESSAGES_BUFFER_SIZE;
            }

            return 0;
        }

        /**
         * @brief Check if the Message Buffer is full
         * 
         * @return true 
         * @return false 
         */
        bool full() {
            return _full;
        }

        /**
         * @brief Get the Message ID Prior x elements before the head  !!! No Boundary check !!!
         * 
         * @param messagesPrior 
         * @return messages_counter_t 
         */
        messages_counter_t getMessageIdPrior(messages_counter_t messagesPrior) {
            messages_counter_t id;

            if (messagesPrior > headMessageId) {
                id = 256 - (messagesPrior - headMessageId);
            } else {
                id = headMessageId - messagesPrior;
            }

            return id;
        }

        /**
         * @brief Get the Count between messageId and the current headMessageId
         * 
         * @param messageId 
         * @return messages_counter_t 
         */
        messages_counter_t distanceFromHeadId(messages_counter_t messageId) {
            if (messageId > headMessageId) {
                return headMessageId + (256 - messageId);
            }

            return headMessageId - messageId;
        }

    protected:
        CarMessage messageBuffer[STD_SYNC_MESSAGES_BUFFER_SIZE];
        messages_counter_t head = 0;
        messages_counter_t tail = 0;
        bool _full = false;

        messages_counter_t headMessageId = 0;

        bool push(CarMessage carMessage) {
            if (full()) return false;

            // Add CarMessage
            messageBuffer[head] = carMessage;

            // Increment counters
            addToCounter(head, 1);
            headMessageId++;

            if (head == tail)
                _full = true;

            return true;
        }

        /**
         * @brief Get the Count between nr and the head
         * 
         * @param nr
         * @return messages_counter_t 
         */
        messages_counter_t distanceFromHead(messages_counter_t nr) {
            if (nr > head) {
                return headMessageId + (STD_SYNC_MESSAGES_BUFFER_SIZE - nr);
            }

            return headMessageId - nr;
        }

        /**
         * @brief Get the Message Nr prior x elements of the head  !!! No Boundary check !!!
         * 
         * @param messagesPrior 
         * @return messages_counter_t 
         */
        messages_counter_t getMessageNrPrior(messages_counter_t messagesPrior) {
            messages_counter_t nr;

            if (messagesPrior > head) {
                nr = STD_SYNC_MESSAGES_BUFFER_SIZE - (messagesPrior - head);
            } else {
                nr = head - messagesPrior;
            }

            return nr;
        }

        /**
         * @brief Convert a messageId to its index in the array. It is calculated and faster than just iterating over the array.  !!! No Boundary check !!!
         * 
         * @param messageId 
         * @return messages_counter_t 
         */
        messages_counter_t messageIdToNr(messages_counter_t messageId) {
            messages_counter_t messageCount = 0;

            if (headMessageId > messageId) {
                messageCount = headMessageId - messageId;
            } else if (headMessageId < messageId) {
                messageCount = 255 - messageId;
                messageCount += (headMessageId + 1);
            }

            return getMessageNrPrior(messageCount);
        }

        /**
         * @brief Check if a message with messageId is in between start and end (endId is exclusiv). Applyable for Nr and Id.
         * 
         * @param startId 
         * @param endId 
         * @return true 
         * @return false 
         */
        bool checkInsideBoundary(messages_counter_t id, messages_counter_t startId, messages_counter_t endId) {
            // Check overflow
            if (startId < endId) {
                // Easy
                if (id >= startId && id < endId)
                    return true;
            } else if (startId > endId) {
                // "Exclude" middle part
                if (id < endId || id >= startId)
                    return true;
            }

            return false;
        }

        /**
         * @brief Check if the Message with messageId has already been added before and is in the buffer
         * 
         * @param messageId 
         * @return true if the message is in the buffer
         * @return false if the message is not in the buffer
         */
        inline bool checkInBuffer(messages_counter_t messageId) {
            return checkInsideBoundary(messageId, tail, head);
        }

        /**
         * @brief Add a given Count to a counter (most likely 1)
         * 
         * @param counter head or tail or custom counter
         * @param value amount of count to be added
         */
        void addToCounter(messages_counter_t &counter, messages_counter_t value) {
            value %= STD_SYNC_MESSAGES_BUFFER_SIZE;

            while(value--) {
                if (counter == STD_SYNC_MESSAGES_BUFFER_SIZE - 1) {
                    counter = 0;
                } else {
                    counter++;
                }
            }
        }

        /**
         * @brief Remove a given Count from a counter (most likely 1)
         * 
         * @param counter head or tail or custom counter
         * @param value amout of cout to be removed (has to be positive)
         */
        void removeFromCounter(messages_counter_t &counter, messages_counter_t value) {
            value %= STD_SYNC_MESSAGES_BUFFER_SIZE;

            while(value--) {
                if (counter == 0) {
                    counter = STD_SYNC_MESSAGES_BUFFER_SIZE -1;
                } else {
                    counter--;
                }
            }
        }
};

// ---------------------------------------------------------------------------------------------------------------------------------------

class SyncedOutBuffer : public SyncedBuffer {
    public:
        /**
         * @brief Add a new Message to the Out Buffer
         * 
         * @param carMessage CarMessage which should be added
         * @return true if the Message was added to the buffer
         * @return false if the buffer is full
         */
        bool add(CarMessage &carMessage) {
            if (full()) return false;

            carMessage.setMessageId(headMessageId);
            push(carMessage);

            return true;
        }

        /**
         * @brief Send the next message(s) from the buffer automatically over the given Channel
         * 
         * @param channel The Channel the Message(s) should be send over
         * @return true if one or more messages got send
         * @return false if no message got send (if this state is for longer, channel is maybe dead)
         */
        bool sendNext(IChannel &channel) {
            bool sentAMessage = false;

            // Begin with lost messages
            messages_counter_t lostId;
            while(_lostMessages.peek(lostId)) {
                if (checkInBuffer(lostId)) {
                    if (channel.send(messageBuffer[messageIdToNr(lostId)]))
                        sentAMessage = true;
                    else
                        return sentAMessage;
                }

                _lostMessages.pop(lostId);
            }

            // If no _lostMessages are there anymore, continue with the regular messages
            while(!empty()) {
                if (channel.send(messageBuffer[tail])) {
                    addToCounter(tail, 1);
                    _full = false;
                    sentAMessage = true;
                } else {
                    break;
                }
            }

            return sentAMessage;
        }

        /**
         * @brief Flag a Message as lost
         * 
         * @param messageId ID of the lost Message
         * @return true ID got flagged as lost
         * @return false message not send before (?)
         */
        bool lost(messages_counter_t messageId) {
            if (checkInBuffer(messageId)) {
                _lostMessages.push(messageId);
                return true;
            }

            return false;
        }

        /**
         * @brief Set a Device with its Buffer as disconnected. The Buffer gets reset to its tail.
         * 
         */
        void disconnected() {
            _nextNrToSend = tail;
        }

        /**
         * @brief Confirm all Messages INCLUDING the Message with messageId as received correctly
         * 
         * @param messageId message until and including will be deleted from the buffer
         * @return true messages deleted from buffer and flagged correctly
         * @return false message with this id was not in the buffer (?)
         */
        bool confirm(messages_counter_t messageId) {
            if (checkInBuffer(messageId)) {
                tail = messageIdToNr(messageId);
                addToCounter(tail, 1);
                _full = false;

                messages_counter_t nextToSendDistance = distanceFromHead(_nextNrToSend);
                messages_counter_t tailDistance = distanceFromHead(tail);

                if (nextToSendDistance > tailDistance)
                    _nextNrToSend = tail;

                return true;
            }

            return false;
        }

    private:
        CircularBuffer<messages_counter_t, STD_SYNC_MESSAGES_BUFFER_SIZE, messages_counter_t> _lostMessages;
        messages_counter_t _nextNrToSend = 0;
};

// ---------------------------------------------------------------------------------------------------------------------------------------

class SyncedInBuffer : public SyncedBuffer {
    public:
        /**
         * @brief Add a received Message to the buffer and check for any errors...
         * 
         * @param carMessage received message
         * @param errorChannel the channel over wich a error message should be sent
         * @param blankMessage a blank Message including pre-set Device-IDs
         * @return true if a confirm message was sent
         * @return false if no confirm message got sent
         */
        bool received(CarMessage &carMessage, IChannel &errorChannel, CarMessage &blankMessage, bool broadcast = false) {
            if (carMessage.getMessageId() != headMessageId) {
                // Some messages got lost. Add placeholders, or,
                // if too many messages would be lost and the amout of them is
                // implausible, just forget it

                // First, check if message is in the legit room of possible IDs after head
                if (checkInsideBoundary(carMessage.getMessageId(), _lastConfirmedMessageId, _lastConfirmedMessageId + STD_SYNC_MESSAGES_BUFFER_SIZE)) {
                    // -> got one or more lost messages, add placeholders
                    messages_counter_t lostCounter = 0;
                    while (carMessage.getMessageId() != headMessageId) {
                        // Generate placeholder
                        CarMessage placeholderMessage;
                        placeholderMessage.setMessageId(headMessageId);
                        placeholderMessage.setComponentId(componentId::getComponentId(COMPONENT_SYSTEM, COMPONENT_SYSTEM_SYNC_OK));
                        placeholderMessage.set(SYNC_MESSAGE_COMMAND_LOST_PLACEHOLDER, 0);

                        // Add placeholder
                        push(placeholderMessage);

                        // Increment counters
                        lostCounter++;
                    }

                    // Now, generate lost response
                    CarMessage lostResponseMessage = blankMessage;
                    lostResponseMessage.setMessageId(0); // -> has no ID
                    lostResponseMessage.setComponentId(componentId::getComponentId(COMPONENT_SYSTEM, COMPONENT_SYSTEM_SYNC_OK));
                    lostResponseMessage.setPrioritise(true);

                    if (lostCounter == 1) {
                        lostResponseMessage.setLength(1);
                        if (broadcast) {
                            lostResponseMessage.set(SYNC_MESSAGE_COMMAND_LOST_BROADCAST, 0);
                        } else {
                            lostResponseMessage.set(SYNC_MESSAGE_COMMAND_LOST, 0);
                        }
                        lostResponseMessage.set(headMessageId - lostCounter, 1);
                    } else {
                        lostResponseMessage.setLength(2);
                        if (broadcast) {
                            lostResponseMessage.set(SYNC_MESSAGE_COMMAND_LOST_FIELD_BROADCAST, 0);
                        } else {
                            lostResponseMessage.set(SYNC_MESSAGE_COMMAND_LOST_FIELD, 0);
                        }
                        lostResponseMessage.set(headMessageId - lostCounter, 1);
                        lostResponseMessage.set(headMessageId, 2);
                    }

                    errorChannel.send(lostResponseMessage);
                } /* Check if it is a resent lost message */ else if (!empty()) {
                    if (checkInsideBoundary(carMessage.getMessageId(), messageBuffer[tail].getMessageId(), headMessageId)) {
                        messageBuffer[messageIdToNr(carMessage.getMessageId())] = carMessage;
                    }
                }
            }

            // Add message at the head of the buffer and increment
            if (full()) {
                #if defined(SYNC_SYNCED_IN_BUFFER_DEBUG) && defined(MESSAGE_REPORT)
                    printf("[SyncedInBuffer]@received: Try to add message in full buffer (WTF?)\n");
                #endif
            } else {
                push(carMessage);
            }

            // At the end, check if the buffer is full and a forced OK is needed
            if (full()) {
                return sendIntervalMessage(errorChannel, blankMessage);
            }

            return false;
        }

        /**
         * @brief Send an intervall message, confirming the current Status of the buffer
         * 
         * @param channel The Channel a OK message should be sent over
         * @param blankMessage A blank Message including Device-IDs
         * @return true if the interval message got enqueued into the channel
         * @return false if the interval message couldn't be sent over the channel
         */
        bool sendIntervalMessage(IChannel &channel, CarMessage &blankMessage, bool broadcast = false) {
            messages_counter_t confirmId;

            if (empty()) {
                confirmId = headMessageId - 1;
            } else {
                // Get latest, deliverable message and confirm successfull arrivel
                // Decide wether lastConfirmedId or tail is the start of the search
                messages_counter_t lastConfirmedIdDistance = distanceFromHeadId(_lastConfirmedMessageId);
                messages_counter_t tailDistance = distanceFromHead(tail);
                messages_counter_t searchIndex;

                if (lastConfirmedIdDistance < tailDistance)
                    searchIndex = messageIdToNr(_lastConfirmedMessageId);
                else
                    searchIndex = tail;

                while(searchIndex != head) {
                    if (messageBuffer[searchIndex].getComponentId() == componentId::getComponentId(COMPONENT_SYSTEM, COMPONENT_SYSTEM_SYNC_OK)) {
                        #if defined(SYNC_SYNCED_IN_BUFFER_DEBUG) && defined(MESSAGE_REPORT)
                            if (messageBuffer[searchIndex].get(0) != SYNC_MESSAGE_COMMAND_LOST_PLACEHOLDER)
                                printf("[SyncedInBuffer]@sendIntervalMessage: Inplausible Message Command stored in placeholder!\n");
                        #endif

                        confirmId = messageBuffer[searchIndex].getMessageId() - 1;
                        break;
                    } else {
                        confirmId = messageBuffer[searchIndex].getMessageId();
                    }
                }
            }

            // Now send confirm message
            CarMessage confirmMessage = blankMessage;
            confirmMessage.setMessageId(0); // -> has no ID
            confirmMessage.setComponentId(componentId::getComponentId(COMPONENT_SYSTEM, COMPONENT_SYSTEM_SYNC_OK));
            confirmMessage.setPrioritise(true);
            confirmMessage.setLength(1);

            if (broadcast) {
                confirmMessage.set(SYNC_MESSAGE_COMMAND_CONFIRM_BROADCAST, 0);
            } else {
                confirmMessage.set(SYNC_MESSAGE_COMMAND_CONFIRM, 0);
            }
            confirmMessage.set(confirmId, 1);

            return channel.send(confirmMessage);
        }

        /**
         * @brief Get the next Message in the Buffer to be delivered (should be called with a loop)
         * 
         * @param carMessage empty message to be filled with the received data
         * @return true if the message was written
         * @return false if no message was there/received
         */
        bool deliver(CarMessage &carMessage) {
            // Check for any message is there
            if (empty()) return false;

            // Check for a dummy message by component id and lost message placeholder
            if (messageBuffer[tail].getComponentId() == componentId::getComponentId(COMPONENT_SYSTEM, COMPONENT_SYSTEM_SYNC_OK)) {
                #if defined(SYNC_SYNCED_IN_BUFFER_DEBUG) && defined(MESSAGE_REPORT)
                    if (messageBuffer[tail].get(0) != SYNC_MESSAGE_COMMAND_LOST_PLACEHOLDER)
                        printf("[SyncedInBuffer]@deliver: Inplausible Message Command stored in placeholder!\n");
                #endif

                return false;
            }

            carMessage = messageBuffer[tail];

            addToCounter(tail, 1);
            _full = false;
            return true;
        }

        /**
         * @brief Check if there are known lost messages in the buffer
         * 
         * @return true if there are known lost messages
         * @return false if there is no known lost message
         */
        bool lostMessagesAvailable() {
            // Iterate over the Buffer till a lost message is found
            for (messages_counter_t currentPos = tail; currentPos != head; addToCounter(currentPos, 1)) {
                if (messageBuffer[currentPos].getComponentId() == componentId::getComponentId(COMPONENT_SYSTEM, COMPONENT_SYSTEM_SYNC_OK)) {
                    #if defined(SYNC_SYNCED_IN_BUFFER_DEBUG) && defined(MESSAGE_REPORT)
                        if (messageBuffer[tail].get(0) != SYNC_MESSAGE_COMMAND_LOST_PLACEHOLDER)
                            printf("[SyncedInBuffer]@lostMessagesAvailable: Inplausible Message Command stored in placeholder!\n");
                    #endif

                    return true;
                }
            }

            return false;
        }

    private:
        CircularBuffer<messages_counter_t, STD_SYNC_MESSAGES_BUFFER_SIZE, messages_counter_t> _lostMessages;
        messages_counter_t _lastConfirmedMessageId = 0 - 1;
};



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
                    if (carMessage.getComponentId() == COMPONENT_SYSTEM_SYNC_OK) {
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
                        if (receiverId == DEVICE_ALL) {
                            if (device.inBroadcastBuffer.received(carMessage, *device.channel, blankMessage, true))
                                device.resetSentIntervalTimer();
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

            if (idFound) {
                carMessage.setSenderId(_thisId);
                carMessage.setReceiverId(receiverDeviceId);

                // Search for receiver device in device list
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
            // Go throu all device buffers
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

                // Last, check for interval message need
                if (device.intervalMessageNeeded()) {
                    bool intervalMessageSent = false;
                    intervalMessageSent |= device.inBuffer.sendIntervalMessage(*device.channel, blankMessage, false);
                    intervalMessageSent |= device.inBroadcastBuffer.sendIntervalMessage(*device.channel, blankMessage, true);

                    if (intervalMessageSent)
                        device.resetSentIntervalTimer();
                }
            }


            // Same for the broadcast buffer
            while (_outBroadcastBuffer.sendNext(_broadcastDummyChannel));
        }

        bool messagesInOutBuffer() {
            if (!_outBroadcastBuffer.empty()) return true;

            for (Device &device : devices)
                if (!device.outBuffer.empty())
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
            }

            IChannel *channel;
            id_device_t deviceId;

            // Data for Sending
            SharedTimer lastGotOk; // The time since the last got OK
            SharedTimer lastSentOk;
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

            void resetOkTimer() {
                lastGotOk.reset();
                lastGotOk.start();
            }

            void resetSentIntervalTimer() {
                lastSentOk.reset();
                lastSentOk.start();
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
            for (Device &device : devices) {
                messages_counter_t thisDistance = _outBroadcastBuffer.distanceFromHeadId(device.lastConfirmedBroadcastId);
                if (thisDistance > longestDistance)
                    longestDistance = thisDistance;
            }

            _outBroadcastBuffer.confirm(_outBroadcastBuffer.getMessageIdPrior(longestDistance));
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