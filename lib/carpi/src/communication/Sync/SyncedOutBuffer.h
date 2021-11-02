#pragma once

#include "SyncedBuffer.h"

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
            if (_buffer.full()) return false;

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
            bool messageGotSent = false;
            uint8_t indexFromTailToSend;

            // First, send as lost marked messages
            for (uint8_t i = 0; i < _lostMessages.size();) {
                if (!idToIndexFromTail(_lostMessages.getTailReference(), indexFromTailToSend)) {
                    #if defined(SYNC_DEBUG_ERRORS) && defined(MESSAGE_REPORT)
                        printf("[SyncedOutBuffer]@sendNext: Couldn't convert id to indexFromTail! (1)\n");
                    #endif
                    return messageGotSent;
                }

                if (channel.send(_buffer.getReferenceFromTail(indexFromTailToSend))) {
                    _lostMessages.dropLast();
                    messageGotSent = true;
                } else {
                    return messageGotSent;
                }
            }

            // Second, send regular messages
            if (_buffer.empty()) return messageGotSent;
            uint8_t afterHeadMessageId = getHeadMessageId() + 1;
            while(_nextIdToSend != afterHeadMessageId) {
                if (!idToIndexFromTail(_nextIdToSend, indexFromTailToSend)) {
                    #if defined(SYNC_DEBUG_ERRORS) && defined(MESSAGE_REPORT)
                        printf("[SyncedOutBuffer]@sendNext: Couldn't convert id to indexFromTail! (2)\n");
                    #endif
                    return messageGotSent;
                }

                if (channel.send(_buffer.getReferenceFromTail(indexFromTailToSend))) {
                    ++_nextIdToSend;
                    messageGotSent = true;
                } else {
                    return messageGotSent;
                }
            }

            return messageGotSent;
        }

        /**
         * @brief Flag a Message as lost
         * 
         * @param messageId ID of the lost Message
         * @return true ID is flagged as lost
         * @return false message not sent before (?)
         */
        bool lost(messages_counter_t messageId) {
            if (containsId(messageId)) {
                if (!checkAlreadyLost(messageId))
                    _lostMessages.push(messageId);
                
                return true;
            }

            return false;
        }

        /**
         * @brief 
         * 
         */

        /**
         * @brief Set a Device with its Buffer as disconnected. All already sent messages get flagged as lost.
         * 
         * @return true one or more messages got flagged as lost
         * @return false no message got flagged as lost
         */
        bool disconnected() {
            bool returnVal = false;
            uint8_t lastSentIndex;

            if (idToIndexFromTail(_nextIdToSend - 1, lastSentIndex)) return false;

            // Mark all messages already sent as lost
            for (uint8_t i = 0; i <= lastSentIndex; ++i) {
                returnVal |= lost(_buffer.getReferenceFromTail(i).getMessageId());
            }

            return returnVal;
        }

        /**
         * @brief Confirm all Messages INCLUDING the Message with messageId as received correctly
         * 
         * @param messageId message until and including will be deleted from the buffer
         * @return true messages deleted from buffer and flagged correctly
         * @return false message with this id was not in the buffer (?)
         */
        bool confirm(messages_counter_t messageId) {
            if (!containsId(messageId)) return false;

            // Then, confirm messages -> shrink down buffer
            uint8_t messageAmountToDelete;
            if (!idToIndexFromTail(messageId, messageAmountToDelete)) return false;

            _buffer.deleteAmountFromTail(messageAmountToDelete + 1); // +1 because index 0 == 1 message to delete because the inclding factor
            
            // Remove leftover lost messages
            for (uint8_t i = 0; i < _lostMessages.size(); ) {
                uint8_t currentId;
                _lostMessages.getFromTail(i, currentId);

                if (containsId(currentId)) {
                    ++i;
                } else {
                    _lostMessages.deleteFromTail(i);
                }
            }

            return true;
        }

        messages_counter_t pendingCount() const {
            return _buffer.size();
        }

    private:
        AdvancedCircularBuffer<uint8_t, STD_SYNCED_BUFFER_MESSAGES_BUFFER_SIZE, uint8_t> _lostMessages;
        uint8_t _nextIdToSend = 0;

        bool checkAlreadyLost(uint8_t id) const {
            for (uint8_t i = 0; i < _lostMessages.size(); i++) {
                uint8_t currentId;
                _lostMessages.getFromTail(i, currentId);

                if (id == currentId) return true;
            }

            return false;
        }
};