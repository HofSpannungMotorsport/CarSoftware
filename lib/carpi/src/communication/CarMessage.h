#ifndef CARMESSAGE_H
#define CARMESSAGE_H

#include "deviceIds.h"
#include "componentIds.h"

#define STD_CARMESSAGE_DATA_SIZE 7

class CarMessage {
    public:
        CarMessage() {}

        // ----------------- Data-related stuff -----------------

        /*
            Get the data at a given position

            @param index The index for the wanted data
            @return Returns the Value at the given index
        */
        uint8_t get(uint8_t index) {
            if (index >= STD_CARMESSAGE_DATA_SIZE)
                return 0;

            return _data[index];
        }

        /*
            Set the data at a given position

            @param value The value which should be written into the data field
            @param index The index for the data to be written to
        */
        void set(uint8_t value, uint8_t index) {
            if (index >= STD_CARMESSAGE_DATA_SIZE)
                return;
            
            _data[index] = value;
        }

        /*
            Directly access the underlying Data-Array of the Message

            @param index The index for the data which should be accessed
            @return Returns a reference to the data at the given index
        */
        uint8_t &operator[](uint8_t index) {
            return _data[index];
        }

        /*
            @return Returns the current set Length of the Message
        */
        uint8_t getLength() {
            return _length;
        }

        /*
            Set the length of the CarMessage data

            @param length The new Length for the Data of the Message
        */
        void setLength(uint8_t length) {
            if (length > STD_CARMESSAGE_DATA_SIZE)
                return;

            _length = length;
        }


        // ------------------- Message header -------------------

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

        void setPrioritise(bool prioritise) {
            _prioritise = prioritise;
        }

        bool getPrioritise() {
            bool _prioritise;
        }

        // returns 0 if messages are equal
        int compareTo(CarMessage &carMessage) {
            // Compare CarMessage Header
            if (_senderId != carMessage.getSenderId())
                return 1;
            if (_receiverId != carMessage.getReceiverId())
                return 2;
            if (_componentId != carMessage.getComponentId())
                return 3;
            
            // Compare CarMessage Data
            if (_length != carMessage.getLength())
                return -1;

            for (uint16_t i = 0; i < _length; i++) {
                if (_data[i] != carMessage.get(i))
                    return -2 -i;
            }

            return 0;
        }
    
    protected:
        id_device_t _senderId = DEVICE_NOT_SET; // only 5 bits are useable
        id_device_t _receiverId = DEVICE_NOT_SET; // only 5 bits are usable

        id_component_t _componentId = 0;

        bool _prioritise = false;

        uint8_t _length = 0;
        uint8_t _data[STD_CARMESSAGE_DATA_SIZE];
};

#endif