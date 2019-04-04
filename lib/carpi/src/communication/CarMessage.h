#ifndef CARMESSAGE_H
#define CARMESSAGE_H

#include <vector>
#include <stdint.h>
#include "deviceIds.h"
#include "componentIds.h"

struct car_sub_message_t {
    uint8_t length;
    uint8_t data[7];
};

class CarMessage : private NonCopyable<CarMessage> {
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
            Get the whole Message Header including Sender ID and Reveiver ID
        */
        id_message_header_t getMessageHeader() {
            return deviceId::getMessageHeader(_senderId, _receiverId);
        }

        /*
            Add a new subMessage to the whole Message.
            Returns true if the message was successfully added.
        */
        bool addSubMessage(car_sub_message_t &subMessage) {
            if (subMessages.size() == subMessages.max_size()) return false;

            subMessages.push_back(subMessage);

            return true;
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
        void setSendPriority(uint8_t sendPriority) {
            _sendPriority = sendPriority;
        }

        /*
            Get the send Priority. Should be a Value between 0 and 255
        */
        uint8_t getSendPriority() {
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
            _sentTimer.reset();
            _sentTimer.start();
        }

        /*
            Get the time passed since the message has been sent
        */
        float getTimeSinceSent() {
            return _sentTimer.read();
        }

        std::size_t getSize() {
            return subMessages.size();
        }
    
    protected:
        id_device_t _senderId; // only 5 bits are useable
        id_device_t _receiverId; // only 5 bits are usable

        id_component_t _componentId;
        
        uint8_t _sendPriority;
        float _timeout;
        
        Timer _sentTimer;
};

#endif