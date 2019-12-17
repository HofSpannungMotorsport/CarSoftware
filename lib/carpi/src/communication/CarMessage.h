#ifndef CARMESSAGE_H
#define CARMESSAGE_H

#include "deviceIds.h"
#include "componentIds.h"

#define STD_CARMESSAGE_DATA_SIZE 7

class CarMessage {
    public:
        CarMessage() {}

        // ----------------- Data-related stuff -----------------

        /**
         * @brief Get the data at a given position
         * 
         * @param index The index for the wanted data
         * @return uint8_t Returns the Value at the given index
         */
        uint8_t get(uint8_t index) {
            if (index >= STD_CARMESSAGE_DATA_SIZE)
                return 0;

            return _data[index];
        }

        /**
         * @brief Set the data at a given position
         * 
         * @param value The value which should be written into the data field
         * @param index The index for the data to be written to
         */
        void set(uint8_t value, uint8_t index) {
            if (index >= STD_CARMESSAGE_DATA_SIZE)
                return;
            
            _data[index] = value;
        }

        /**
         * @brief Directly access the underlying Data-Array of the Message
         * 
         * @param index The index for the data which should be accessed
         * @return uint8_t& Returns a reference to the data at the given index
         */
        uint8_t &operator[](uint8_t index) {
            return _data[index];
        }

        /**
         * @brief Get the Length of the CarMessage data
         * 
         * @return uint8_t Returns the current set Length of the Message
         */
        uint8_t getLength() {
            return _length;
        }

        /**
         * @brief Set the length of the CarMessage data
         * 
         * @param length The new Length for the Data of the Message
         */
        void setLength(uint8_t length) {
            if (length > STD_CARMESSAGE_DATA_SIZE)
                return;

            _length = length;
        }


        // ------------------- Message header -------------------

        /**
         * @brief Set the senderId for the device which sends the message
         * 
         * @param senderId Set the senderId for this message
         */
        void setSenderId(id_device_t senderId) {
            _senderId = senderId;
        }

        /**
         * @brief Get the senderId for the device which sends the message
         * 
         * @return id_device_t Returns the senderId for this message
         */
        id_device_t getSenderId() {
            return _senderId;
        }

        /**
         * @brief Set the receiverId for the device which should reveive the message
         * 
         * @param receiverId Set the receiverId for this message
         */
        void setReceiverId(id_device_t receiverId) {
            _receiverId = receiverId;
        }

        /**
         * @brief Get the receiverId for the device which should reveive the message
         * 
         * @return id_device_t Returns the receiverId for this message
         */
        id_device_t getReceiverId() {
            return _receiverId;
        }

        /**
         * @brief Set the Message ID (important for sending). Should be set automatically by the syncer
         * 
         * @param messageId The new Message ID for the Message (uint8_t)
         */
        void setMessageId(uint8_t messageId) {
            _messageId = messageId;
        }

        /**
         * @brief Get the Message ID of this message
         * 
         * @return uint8_t Returns the Message ID of this Message
         */
        uint8_t getMessageId() {
            return _messageId;
        }

        /**
         * @brief Set the Sender and Receiver Id from a message header
         * 
         * @param messageHeader The message header including SenderID and ReceiverID
         */
        void setIdsFromMessageHeader(id_message_header_t messageHeader) {
            setSenderId(deviceId::getDeviceIdFromMessageHeader(messageHeader, DEVICE_TYPE_SENDER));
            setReceiverId(deviceId::getDeviceIdFromMessageHeader(messageHeader, DEVICE_TYPE_RECEIVER));
        }

        /**
         * @brief Get the whole Message ID Header including Sender ID and Reveiver ID
         * 
         * @return id_message_header_t Returns the message header with senderId and receiverId
         */
        id_message_header_t getMessageHeader() {
            return deviceId::getMessageHeader(_senderId, _receiverId);
        }

        /**
         * @brief Set the Component ID the message is dedicated to
         * 
         * @param componentId The Component ID this message is dedicated to
         */
        void setComponentId(id_component_t componentId) {
            _componentId = componentId;
        }

        /**
         * @brief Get the Component ID the message is dedicated to
         * 
         * @return id_component_t Returns the Component ID this message is dedicated to
         */
        id_component_t getComponentId() {
            return _componentId;
        }

        /**
         * @brief Set if this Message should be prioritised before others on a given Channel
         * This means, the Message is sent faster then other messages of other devices
         * on the same Bus
         * 
         * @param prioritise prioritise Set wether this message should be prioritised or not
         */
        void setPrioritise(bool prioritise) {
            _prioritise = prioritise;
        }

        /**
         * @brief Get the current Prioritise status of this message
         * 
         * @return Returns the current Prioritise status
         */
        bool getPrioritise() {
            bool _prioritise;
        }

        /**
         * @brief Compare a CarMessage against this CarMessage
         * 
         * @param carMessage The CarMessage this should be compared to
         * @return int Returns the comparison result
         */
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

        uint8_t _messageId = 0;

        id_component_t _componentId = 0;

        bool _prioritise = false;

        uint8_t _length = 0;
        uint8_t _data[STD_CARMESSAGE_DATA_SIZE];
};

#endif