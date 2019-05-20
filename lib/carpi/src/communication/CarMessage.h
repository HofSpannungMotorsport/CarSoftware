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

enum car_message_dropable_t : bool {
    IS_NOT_DROPABLE = false,
    IS_DROPABLE = true
};

typedef uint8_t car_message_priority_t;

#include "SendPriority.h"

#define CAR_MESSAGE_PRIORITY_LOWEST 255
#define CAR_MESSAGE_PRIORITY_HIGHEST 0

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
            _receiverId = receiverId;
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

        void setDropable(car_message_dropable_t dropable) {
            _dropable = dropable;
        }

        car_message_dropable_t getDropable() {
            return _dropable;
        }
    
    protected:
        id_device_t _senderId = DEVICE_NOT_SET; // only 5 bits are useable
        id_device_t _receiverId = DEVICE_NOT_SET; // only 5 bits are usable

        id_component_t _componentId = 0;

        car_message_priority_t _sendPriority = CAR_MESSAGE_PRIORITY_LOWEST;

        /*
            If a message is send often and repeadly, it could let overflow the outgoing bessage queue.
            These messages are not so importent and can be dropped because a new message will come really soon,
            being more uptodate and making the last message useless.

            A Configuration-Message is sent mostly only once and should not be dropped at all!
        */
        car_message_dropable_t _dropable = IS_NOT_DROPABLE;
};

#endif