#ifndef CARMESSAGE_H
#define CARMESSAGE_H

#include "deviceIds.h"
#include "componentIds.h"

#define STD_CARMESSAGE_DATA_SIZE 7 // Max is 8, currently only 7 is needed

/*
    The CarMessage is as it is. No Changes should be made in the future for
    compatibility reasons with all Channels.

    Please keep in mind, that the Prioritise-Bit may be sacrificed for a
    larger ComponentID in the future!
*/

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
        uint8_t get(uint8_t index) const {
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
         * @brief Get the Length of the CarMessage data
         * 
         * @return uint8_t Returns the current set Length of the Message
         */
        inline uint8_t getLength() const {
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
         * @brief Set the senderId for the device which sends the message. Normally set by the Syncer
         * 
         * @param senderId Set the senderId for this message
         */
        inline void setSenderId(id_device_t senderId) {
            _senderId = senderId;
        }

        /**
         * @brief Get the senderId for the device which sends the message
         * 
         * @return id_device_t Returns the senderId for this message
         */
        inline id_device_t getSenderId() const {
            return _senderId;
        }

        /**
         * @brief Set the receiverId for the device which should receive the message. Normally set by the Syncer
         * 
         * @param receiverId Set the receiverId for this message
         */
        inline void setReceiverId(id_device_t receiverId) {
            _receiverId = receiverId;
        }

        /**
         * @brief Get the receiverId for the device which should reveive the message
         * 
         * @return id_device_t Returns the receiverId for this message
         */
        inline id_device_t getReceiverId() const {
            return _receiverId;
        }

        /**
         * @brief Set the Message ID (important for sending). Is automatically set by the Syncer
         * 
         * @param messageId The new Message ID for the Message (uint8_t)
         */
        inline void setMessageId(uint8_t messageId) {
            _messageId = messageId;
        }

        /**
         * @brief Get the Message ID of this message
         * 
         * @return uint8_t Returns the Message ID of this Message
         */
        inline uint8_t getMessageId() const {
            return _messageId;
        }

        /**
         * @brief Set the Component ID the message is dedicated to. Must be set by the component (SelfSyncable does this automatically).
         * 
         * @param componentId The Component ID this message is dedicated to
         */
        inline void setComponentId(id_component_t componentId) {
            _componentId = componentId;
        }

        /**
         * @brief Get the Component ID the message is dedicated to
         * 
         * @return id_component_t Returns the Component ID this message is dedicated to
         */
        inline id_component_t getComponentId() const {
            return _componentId;
        }

        /**
         * @brief Set if this Message should be prioritised before others on a given Channel
         * This means, the Message is sent faster then other messages of other devices
         * on the same Bus. Normally set by the Syncer automatically
         * 
         * @param prioritise prioritise Set wether this message should be prioritised or not
         */
        inline void setPrioritise(bool prioritise) {
            _prioritise = prioritise;
        }

        /**
         * @brief Get the current Prioritise status of this message
         * 
         * @return Returns the current Prioritise status
         */
        inline bool getPrioritise() const {
            return _prioritise;
        }


        // ---------------------- Compare -----------------------

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