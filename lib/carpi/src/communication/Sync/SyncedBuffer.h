#pragma once

#include "../../steroido/src/Common/AdvancedCircularBuffer.h"

class SyncedBuffer {
    public:

        /**
         * @brief Returns true if a Message with the given ID is in the Buffer
         * 
         * @param id 
         * @return true 
         * @return false 
         */
        bool containsId(uint8_t id) const {
            if (_buffer.empty()) return false;

            bool containing = false;
            uint8_t headId = _headMessageId; // The ID of the newest Message in the Container
            uint8_t tailId = _headMessageId - _buffer.size() + 1; // The ID of the tail/oldest Message

            if (headId >= tailId) {
                if (id <= headId && id >= tailId)
                    containing = true;
            } else if (headId < tailId) {
                if (id <= headId || id >= tailId)
                    containing = true;
            }

            return containing;
        }

        bool idToIndexFromHead(uint8_t id, uint8_t& index) const {
            if (!containsId(id)) return false;

            index = getIdDistance(id, _headMessageId);

            return true;
        }

        bool idToIndexFromTail(uint8_t id, uint8_t& index) const {
            if (!containsId(id)) return false;

            index = getIdDistance(_headMessageId - _buffer.size() + 1, id);

            return true;
        }

        uint8_t getIdDistance(uint8_t tailId, uint8_t headId) const {
            if (tailId <= headId) {
                return headId - tailId;
            } else {
                uint8_t headDistance = headId + 1;
                uint8_t tailDistance = 255 - tailId;

                return headDistance + tailDistance;
            }
        }

        bool possibleFuture(uint8_t oldestId, uint8_t id) const {
            if (getIdDistance(oldestId, id) < STD_SYNCED_BUFFER_MESSAGES_BUFFER_SIZE)
                return true;
            
            return false;
        }

        inline uint8_t getHeadMessageId() {
            return _headMessageId;
        }

    protected:
        AdvancedCircularBuffer<CarMessage, STD_SYNCED_BUFFER_MESSAGES_BUFFER_SIZE, uint8_t> _buffer;

        void push(CarMessage& carMessage) {
            carMessage.setMessageId(++_headMessageId);
            _buffer.push(carMessage);
        }
    
    private:
        /**
         * @brief The ID of the Message in the Head -> NOT the element behind head!
         * 
         */
        uint8_t _headMessageId = 0 - 1;
};