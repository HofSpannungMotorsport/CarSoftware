#pragma once

#include "SyncedBuffer.h"

class SyncedInBuffer : public SyncedBuffer {
    public:
        SyncedInBuffer() {
            _lastAlmostFullMessage.start();
        }

        /**
         * @brief Add a received Message to the buffer and check for any errors...
         * 
         * @param carMessage received message
         * @param replyChannel the channel over which an error or confirm message should be sent
         * @param blankMessage a blank Message including pre-set Device-IDs
         * @return true if a confirm message was sent
         * @return false if no confirm message got sent
         */
        bool received(CarMessage &carMessage, IChannel &replyChannel, CarMessage &blankMessage, bool broadcast = false) {
            bool returnVal = false;
            uint8_t messageId = carMessage.getMessageId();
            uint8_t indexFromHead;
            
            // First check if already got the message -> Maybe a resent lost message?
            if (idToIndexFromHead(messageId, indexFromHead)) {
                // Just overwrite the Message in the buffer -> faster than checking and maybe overwriting the one in the buffer
                _buffer.getReferenceFromHead(indexFromHead) = carMessage;
            }

            // Or the Message is one in the Future
            else if (possibleFuture(getHeadMessageId() - _buffer.size() + 1, messageId)) {
                // Now the message can be the direct following of the last correctly got one
                // OR there are lost messages between -> Add placeholder and send lost message
                uint8_t afterHeadMessageId = getHeadMessageId() + 1;
                if (messageId != afterHeadMessageId) { // -> need to add at least 1 placeholder message
                    // Prepare Placeholder
                    CarMessage placeholderMessage;
                    id_component_t systemComponentId = componentId::getComponentId(COMPONENT_SYSTEM, COMPONENT_SYSTEM_SYNC_OK);

                    placeholderMessage.setComponentId(systemComponentId);
                    placeholderMessage.set(SYNC_MESSAGE_COMMAND_LOST_PLACEHOLDER, 0);

                    // Note for lost reply
                    uint8_t firstLostMessage = getHeadMessageId() + 1;
                    uint8_t currentLostMessage = getHeadMessageId();

                    while (messageId != afterHeadMessageId) {
                        push(placeholderMessage);
                        ++currentLostMessage;
                        afterHeadMessageId = getHeadMessageId() + 1;
                    }


                    // Send lost message reply
                    if (firstLostMessage == currentLostMessage) {
                        // Only one Message got lost -> send a single lost message reply
                        sendLost(currentLostMessage, replyChannel, blankMessage, broadcast);
                    } else {
                        // More than one Message got lost -> send a lost message field reply
                        sendLostField(firstLostMessage, currentLostMessage, replyChannel, blankMessage, broadcast);
                    }
                }

                push(carMessage);

                // At last, send confirm message if needed
                if (_buffer.leftCapacity() <= STD_SYNCED_IN_BUFFER_ALMOST_FULL_LEFT_CAPACITY) {
                    if (_lastAlmostFullMessage.read_ms() >= STD_SYNCED_IN_BUFFER_ALMOST_FULL_MESSAGE_COOLDOWN) {
                        _lastAlmostFullMessage.reset();
                        _lastAlmostFullMessage.start();

                        returnVal |= sendConfirm(replyChannel, blankMessage, broadcast);
                    }
                }
            }

            // Last, may echo an error/notice
            #if defined(SYNC_DEBUG_IN_BUFFER_DROPPED_MESSAGES) && defined(MESSAGE_REPORT)
            else {
                printf("[SyncedInBuffer]@received: Dropped received message with ID %i!\n", messageId);
            }
            #endif

            return returnVal;
        }

        /**
         * @brief Send an intervall message, confirming the current Status of the buffer
         * 
         * @param replyChannel The Channel a OK message should be sent over
         * @param blankMessage A blank Message including Device-IDs
         * @return true if the interval message got enqueued into the replyChannel
         * @return false if the interval message couldn't be sent over the replyChannel
         */
        bool sendIntervalMessage(IChannel &replyChannel, CarMessage &blankMessage, bool broadcast = false) {
            bool returnVal = false;
            id_component_t systemComponentId = componentId::getComponentId(COMPONENT_SYSTEM, COMPONENT_SYSTEM_SYNC_OK);
            
            // Check for lost Messages and resend lost message
            // Iterate through the buffer and either produce a single lost message(s) or a lost field(s)
            bool inLostField = false;
            uint8_t lostFieldStart;
            uint8_t lostFieldEnd;
            for (uint8_t i = 0; i < _buffer.size(); ++i) {
                CarMessage& currentMessage = _buffer.getReferenceFromTail(i);
                if (currentMessage.getComponentId() == systemComponentId) {
                    #if defined(SYNC_DEBUG_ERRORS) && defined(MESSAGE_REPORT)
                    if (currentMessage.get(0) != SYNC_MESSAGE_COMMAND_LOST_PLACEHOLDER) {
                            printf("[SyncedInBuffer]@sendIntervalMessage: Wrong lost message placeholder! (1)\n");
                    }
                    #endif

                    lostFieldEnd = currentMessage.getMessageId();
                    
                    if (!inLostField) {
                        inLostField = true;
                        lostFieldStart = lostFieldEnd;
                    }
                } else if (inLostField) {
                    inLostField = false;

                    if (lostFieldStart == lostFieldEnd) {
                        sendLost(lostFieldEnd, replyChannel, blankMessage, broadcast);
                    } else {
                        sendLostField(lostFieldStart, lostFieldEnd, replyChannel, blankMessage, broadcast);
                    }
                }
            }

            if (inLostField) {
                if (lostFieldStart == lostFieldEnd) {
                    sendLost(lostFieldEnd, replyChannel, blankMessage, broadcast);
                } else {
                    sendLostField(lostFieldStart, lostFieldEnd, replyChannel, blankMessage, broadcast);
                }
            }

            // Confirm current tail
            returnVal = sendConfirm(replyChannel, blankMessage, broadcast);

            return returnVal;
        }

        /**
         * @brief Get the next Message in the Buffer to be delivered (should be called with a loop)
         * 
         * @param carMessage empty message to be filled with the received data
         * @return true if the message was written
         * @return false if no message was there/received or there are lost messages to be received until next can be delivered
         */
        bool deliver(CarMessage &carMessage) {
            if (_buffer.empty()) return false;

            CarMessage& carMessageRef = _buffer.getTailReference();
            id_component_t systemComponentId = componentId::getComponentId(COMPONENT_SYSTEM, COMPONENT_SYSTEM_SYNC_OK);

            if (carMessageRef.getComponentId() == systemComponentId) {
                #if defined(SYNC_DEBUG_ERRORS) && defined(MESSAGE_REPORT)
                if (carMessageRef.get(0) != SYNC_MESSAGE_COMMAND_LOST_PLACEHOLDER) {
                        printf("[SyncedInBuffer]@deliver: Wrong lost message placeholder! (2)\n");
                }
                #endif

                return false;
            }

            return _buffer.pop(carMessage);
        }

        /**
         * @brief Check if there are known lost messages in the buffer
         * 
         * @return true if there are known lost messages
         * @return false if there is no known lost message
         */
        bool lostMessagesAvailable() {
            uint8_t currentSize = _buffer.size();
            id_component_t searchComponentId = componentId::getComponentId(COMPONENT_SYSTEM, COMPONENT_SYSTEM_SYNC_OK);

            for(uint8_t i = 0; i < currentSize; ++i) {
                CarMessage& carMessage = _buffer.getReferenceFromTail(i);

                if (carMessage.getComponentId() == searchComponentId) {
                    #if defined(SYNC_DEBUG_ERRORS) && defined(MESSAGE_REPORT)
                    if (carMessage.get(0) != SYNC_MESSAGE_COMMAND_LOST_PLACEHOLDER) {
                            printf("[SyncedInBuffer]@lostMessagesAvailable: Wrong lost message placeholder!");
                    }
                    #endif

                    return true;
                }
            }

            return false;
        }

    private:
        uint8_t _lastConfirmedId = 255;
        SharedTimer _lastAlmostFullMessage;

        /**
         * @brief Send a lost message to the other syncedBuffer on the distant device
         * 
         * @param lostId the ID of the lost message
         * @param replyChannel the Channel the reply should be sent over
         * @param blankMessage a blank Message including pre-set Device-IDs
         * @param broadcast true if this is a broadcast message, false if not
         */
        void sendLost(uint8_t lostId, IChannel& replyChannel, CarMessage& blankMessage, bool broadcast) {
            CarMessage errorMessage = blankMessage;
            errorMessage.setComponentId(componentId::getComponentId(COMPONENT_SYSTEM, COMPONENT_SYSTEM_SYNC_OK));

            errorMessage.setLength(2);

            if (broadcast)
                errorMessage.set(SYNC_MESSAGE_COMMAND_LOST_BROADCAST, 0);
            else
                errorMessage.set(SYNC_MESSAGE_COMMAND_LOST, 0);
                        
            errorMessage.set(lostId, 1);

            replyChannel.send(errorMessage);
        }

        /**
         * @brief Send a lost message field to the other syncedBuffer on the distant device
         * 
         * @param lostStartId the (including) first ID of the first lost message in the field
         * @param lostEndId the (including) last ID of the last lost message in the field
         * @param replyChannel the Channel the reply should be sent over
         * @param blankMessage a blank Message including pre-set Device-IDs
         * @param broadcast true if this is a broadcast message, false if not
         */
        void sendLostField(uint8_t lostStartId, uint8_t lostEndId, IChannel& replyChannel, CarMessage& blankMessage, bool broadcast) {
            CarMessage errorMessage = blankMessage;
            errorMessage.setComponentId(componentId::getComponentId(COMPONENT_SYSTEM, COMPONENT_SYSTEM_SYNC_OK));
            
            errorMessage.setLength(3);

            if (broadcast)
                errorMessage.set(SYNC_MESSAGE_COMMAND_LOST_FIELD_BROADCAST, 0);
            else
                errorMessage.set(SYNC_MESSAGE_COMMAND_LOST_FIELD, 0);
                        
            errorMessage.set(lostStartId, 1);
            errorMessage.set(lostEndId, 2);

            replyChannel.send(errorMessage);
        }

        /**
         * @brief 
         * 
         * @param replyChannel the Channel the reply should be sent over
         * @param blankMessage a blank Message including pre-set Device-IDs
         * @param broadcast true if this is a broadcast message, false if not
         * @return true if the interval message got enqueued into the replyChannel
         * @return false if the interval message couldn't be sent over the replyChannel
         */
        bool sendConfirm(IChannel& replyChannel, CarMessage& blankMessage, bool broadcast) {
            bool returnVal = false;
            
            uint8_t idToBeConfirmed = getHeadMessageId() - _buffer.size();
            
            CarMessage replyMessage = blankMessage;
            replyMessage.setLength(2);
            replyMessage.setComponentId(componentId::getComponentId(COMPONENT_SYSTEM, COMPONENT_SYSTEM_SYNC_OK));

            if (broadcast)
                replyMessage.set(SYNC_MESSAGE_COMMAND_CONFIRM_BROADCAST, 0);
            else
                replyMessage.set(SYNC_MESSAGE_COMMAND_CONFIRM, 0);

            replyMessage.set(idToBeConfirmed, 1);

            returnVal = replyChannel.send(replyMessage);

            if (returnVal)
                _lastConfirmedId = idToBeConfirmed;

            return returnVal;
        }
};