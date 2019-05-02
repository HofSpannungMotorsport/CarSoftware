#ifndef CARMESSAGE_H
#define CARMESSAGE_H

#include <memory>
#include <vector>
#include <stdint.h>
#include "deviceIds.h"
#include "componentIds.h"

using namespace std;

struct car_sub_message_t {
    uint8_t length;
    uint8_t data[7];
};

typedef uint8_t car_message_priority_t;

#include "SendPriority.h"

#define CAR_MESSAGE_PRIORITY_HIGHEST 255
#define CAR_MESSAGE_PRIORITY_LOWEST 0

class CarMessage {
    public:
        CarMessage() {}

        /*
            Only use for reading the Messages, for Writing use addSubMessage()
        */
        vector<car_sub_message_t> subMessages;

        /*
            Set the senderId for the device which sends the message
        */
        void setSenderId(id_device_t senderId) {
            _senderId = senderId;
        }

        /*
            Get the senderId for the device which sends the message
        */
        id_device_t getSenderId() {
            return _senderId;
        }

        /*
            Set the receiverId for the device which should reveive the message
        */
        void setReceiverId(id_device_t receiverId) {
            _receiverId = _receiverId;
        }

        /*
            Get the receiverId for the device which should reveive the message
        */
        id_device_t getReceiverId() {
            return _receiverId;
        }

        /*
            Seth the Sender and Receiver Id from a message header
        */
        void setIdsFromMessageHeader(id_message_header_t messageHeader) {
            setSenderId(deviceId::getDeviceIdFromMessageHeader(messageHeader, DEVICE_TYPE_SENDER));
            setReceiverId(deviceId::getDeviceIdFromMessageHeader(messageHeader, DEVICE_TYPE_RECEIVER));
        }

        /*
            Get the whole Message Header including Sender ID and Reveiver ID
        */
        id_message_header_t getMessageHeader() {
            return deviceId::getMessageHeader(_senderId, _receiverId);
        }

        /*
            Add a new subMessage to the whole Message.
        */
        void addSubMessage(car_sub_message_t &subMessage) {
            subMessages.push_back(subMessage);
        }

        /*
            Remove first subMessage
        */
        void removeFirstSubMessage() {
            subMessages.erase(subMessages.begin());
        }

        /*
            Set the Component ID the message is dedicated to
        */
        void setComponentId(id_component_t componentId) {
            _componentId = componentId;
        }

        /*
            Get the Component ID the message is dedicated to
        */
        id_component_t getComponentId() {
            return _componentId;
        }

        /*
            Set the send Priority. Should be a Value between 1 and 255
        */
        void setSendPriority(car_message_priority_t sendPriority) {
            _sendPriority = sendPriority;
        }

        /*
            Get the send Priority. Should be a Value between 0 and 255
        */
        car_message_priority_t getSendPriority() {
            return _sendPriority;
        }

        /*
            Set the Timeout for a message, e.g. define the max. time a message can sit in the queue
        */
        void setTimeout(float timeout) {
            _timeout = timeout;
        }

        /*
            Get the Timeout for a message, e.g. get the max. time a message can sit in the queue
        */
        float getTimeout() {
            return _timeout;
        }

        /*
            Start the Timer from when on the message was sent. Should be called shortly before putting the message into the queue.
        */
        void startSentTimer() {
            _sentTimer = std::shared_ptr<Timer>(new Timer());
            _sentTimerSet = true;
            _sentTimer->reset();
            _sentTimer->start();
        }

        /*
            Get the time passed since the message has been sent
        */
        float getTimeSinceSent() {
            if (!_sentTimerSet) return 0;

            return _sentTimer->read();
        }

        /*
            Get the information (true/false) if the message got timed out
        */
        bool timeout() {
            if (!_sentTimerSet) return false;

            return (*_sentTimer >= _timeout);
        }
    
    protected:
        id_device_t _senderId; // only 5 bits are useable
        id_device_t _receiverId; // only 5 bits are usable

        id_component_t _componentId;
        
        car_message_priority_t _sendPriority;
        float _timeout;
        
        std::shared_ptr<Timer> _sentTimer;
        bool _sentTimerSet = false;
};

#endif