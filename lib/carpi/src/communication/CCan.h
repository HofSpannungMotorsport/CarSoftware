#ifndef CCAN_H
#define CCAN_H

#include <vector>
using namespace std;

#include "Sync.h"

#ifndef DEVICE_CAN
    #define DEVICE_CAN // Otherwise a compiler issue (not error) -> defined by mbed before?
#endif

#define STD_CAN_FREQUENCY 250000

#define STD_MAX_OUT_QUEUE_SIZE 100 // Elements
#define STD_MAX_OUT_QUEUE_TIMEOUT_SIZE STD_MAX_OUT_QUEUE_SIZE

#define STD_CAN_TIMEOUT (1.0/(float)STD_CAN_FREQUENCY) // s

class CCan : public IChannel {
    public:
        CCan() = delete;
        CCan(Sync &syncer, PinName rx, PinName tx, int frequency = STD_CAN_FREQUENCY)
        : _syncer(syncer), _can(rx, tx, frequency) {
            _can.attach(callback(this, &CCan::_canMessageReceive), CAN::RxIrq);
        }

        void send(CarMessage &carMessage) {
            // Drop oldest message if _dropQueue is full
            while (_outQueue.size() >= _maxSize.outQueue) {
                _dropOldestMessage(_outQueue);
                _dropped.outQueue++;
            }

            _outQueue.push_back(carMessage);
            _send();
        }
    
    private:
        CAN _can;
        Sync &_syncer;

        float _canTimeout = STD_CAN_TIMEOUT;

        vector<CarMessage> _outQueue;
        vector<CarMessage> _outQueueTimeout;

        struct _dropped {
            uint32_t outQueue = 0;
            uint32_t outQueueTimeout = 0;
        } _dropped;

        struct _maxSize {
            uint16_t outQueue = STD_MAX_OUT_QUEUE_SIZE;
            uint16_t outQueueTimeout = STD_MAX_OUT_QUEUE_TIMEOUT_SIZE; 
        } _maxSize;

        Ticker _sendTicker;

        void _canMessageReceive() {
            CANMessage canMessage;

            while(_can.read(canMessage)) {
                CarMessage carMessage;
                _getCarMessage(canMessage, carMessage);
                _syncer.receive(carMessage);
            }
        }

        /*
            Drops the oldest dropable message.
            Oldest means the last message in the vector containing it.
            -> oldest Message measured on the time it was put in the container, not the duration of the timeout
            -> should be not important anymore because a newer more uptodate message is already present
        */
        void _dropOldestMessage(vector<CarMessage> &queue) {
            for (auto carMessageIterator = queue.begin(); carMessageIterator != queue.end(); carMessageIterator++) {
                if (carMessageIterator->getDropable() == IS_DROPABLE) {
                    queue.erase(carMessageIterator);
                    return;
                }
            }
        }

        void _getCarMessage(CANMessage &canMessage, CarMessage &carMessage) {
            carMessage.setIdsFromMessageHeader(canMessage.id);
            carMessage.setComponentId((id_component_t)canMessage.data[0]);

            #ifdef CCAN_DEBUG
                pcSerial.printf("[CCAN]@_getCarMessage: Made Car Message for component with ID 0x%x (carMessageId: 0x%x)\n", canMessage.data[0], carMessage.getComponentId());
            #endif

            car_sub_message_t subMessage;

            for (uint8_t i = 0; i < canMessage.len-1; i++) {
                subMessage.data[i] = canMessage.data[i+1];
            }

            carMessage.addSubMessage(subMessage);
        }

        void _getCanMessage(CarMessage &carMessage, uint16_t subMessageNumber, CANMessage &canMessage) {
            car_sub_message_t &subMessage = carMessage.subMessages[subMessageNumber];

            canMessage.format = CANStandard;
            canMessage.id = carMessage.getMessageHeader();
            canMessage.len = subMessage.length + 1;
            canMessage.data[0] = carMessage.getComponentId();

            for (uint8_t i = 0; i < subMessage.length; i++) {
                canMessage.data[i+1] = subMessage.data[i];
            }

            #ifdef CCAN_SENDING_DEBUG
                pcSerial.printf("[CCAN]@_getCanMessage: Made CAN Message with component ID 0x%x and message ID 0x%x\n", canMessage.data[0], canMessage.id);
            #endif
        }

        car_message_priority_t _getHighestPriortiy(vector<CarMessage> &queue) {
            car_message_priority_t highestPriorityFound = CAR_MESSAGE_PRIORITY_LOWEST;
            for(CarMessage &carMessage : queue) {
                if(carMessage.getSendPriority() < highestPriorityFound) {
                    highestPriorityFound = carMessage.getSendPriority();
                }
            }

            return highestPriorityFound;
        }

        void _send() {
            // At first, check if there are outdated messages
            for (auto carMessageIterator = _outQueue.begin(); carMessageIterator != _outQueue.end(); ) {
                if (carMessageIterator->timeout()) {
                    // Drop oldest Message if _outQueueTimeout is full
                    while (_outQueueTimeout.size() >= _maxSize.outQueueTimeout) {
                        _dropOldestMessage(_outQueueTimeout);
                        _dropped.outQueueTimeout++;
                    }

                    _outQueueTimeout.push_back(*carMessageIterator);
                    carMessageIterator = _outQueue.erase(carMessageIterator);
                } else {
                    carMessageIterator++;
                }
            }

            // Next, check if there are Messages to send and try to send them if possible
            while(!_outQueue.empty() || !_outQueueTimeout.empty()) {
                if (!_outQueueTimeout.empty()) {
                    // Search highest priority in two steps
                    // First find the highest priority (the smallest priority-number)
                    // then find the first message with this priortiy and send it

                    car_message_priority_t highestPriortiy = _getHighestPriortiy(_outQueueTimeout);

                    for(auto carMessageIterator = _outQueueTimeout.begin(); carMessageIterator != _outQueueTimeout.end(); ) {
                        if (carMessageIterator->getSendPriority() == highestPriortiy) {
                            CANMessage canMessage;
                            _getCanMessage(*carMessageIterator, 0, canMessage);

                            #ifdef CCAN_SENDING_DEBUG
                                pcSerial.printf("[CCan]@_send->_outQueueTimeout: Try to send canMessage with component ID 0x%x and message ID 0x%x\n", canMessage.data[0], canMessage.id);
                            #endif
                            
                            // Now try to send the Message over CAN
                            int canWriteResult = _can.write(canMessage);
                            if (canWriteResult == 1) {
                                // If sent successfully the first subMessage, remove it and look if there are more
                                carMessageIterator->removeFirstSubMessage();
                                if(carMessageIterator->subMessages.empty()) {
                                    // If it was the last/only one, remove the whole Message
                                    carMessageIterator = _outQueueTimeout.erase(carMessageIterator);
                                } else {
                                    // If a message already got sent partly, make it not dropable
                                    carMessageIterator->setDropable(IS_NOT_DROPABLE);
                                }

                                #ifdef CCAN_SENDING_DEBUG
                                    pcSerial.printf("[CCan]@_send->_outQueueTimeout: Successfully sent canMessage with component ID 0x%x and message ID 0x%x\n", canMessage.data[0], canMessage.id);
                                #endif
                            } else {
                                // If the message could't have been sent, try again later (alligator)
                                _sendTicker.attach(callback(this, &CCan::_send), _canTimeout);
                                return;
                            }
                        }
                    }
                } else if (!_outQueue.empty()) {
                    // And Same again, but for the _outQueue.
                    // Search highest priority in two steps
                    // First find the highest priority (the smallest priority-number)
                    // then find the first message with this priortiy and send it

                    car_message_priority_t highestPriortiy = _getHighestPriortiy(_outQueue);

                    for(auto carMessageIterator = _outQueue.begin(); carMessageIterator != _outQueue.end(); ) {
                        if (carMessageIterator->getSendPriority() == highestPriortiy) {
                            CANMessage canMessage;
                            _getCanMessage(*carMessageIterator, 0, canMessage);

                            // Because of the outdated message above, we should make this message less importent by setting the first ID-bit to 1
                            canMessage.id |= 0x400; // -> Binär 10000000000

                            #ifdef CCAN_SENDING_DEBUG
                                pcSerial.printf("[CCan]@_send->_outQueue: Try to send canMessage with component ID 0x%x and message ID 0x%x\n", canMessage.data[0], canMessage.id);
                            #endif
                            
                            // Now try to send the Message over CAN
                            int canWriteResult = _can.write(canMessage);
                            if (canWriteResult == 1) {
                                // If sent successfully the first subMessage, remove it and look if there are more
                                carMessageIterator->removeFirstSubMessage();
                                if(carMessageIterator->subMessages.empty()) {
                                    // If it was the last/only one, remove the whole Message
                                    carMessageIterator = _outQueue.erase(carMessageIterator);
                                } else {
                                    // If a message already got sent partly, make it not dropable
                                    carMessageIterator->setDropable(IS_NOT_DROPABLE);
                                }

                                #ifdef CCAN_SENDING_DEBUG
                                    pcSerial.printf("[CCan]@_send->_outQueue: Successfully sent canMessage with component ID 0x%x and message ID 0x%x\n", canMessage.data[0], canMessage.id);
                                #endif
                            } else {
                                // If the message could't have been sent, try again later (alligator)
                                _sendTicker.attach(callback(this, &CCan::_send), _canTimeout);
                                return;
                            }
                        }
                    }
                }
            }

            // All Messages in the Queue are sent. Detach the Ticker. Over and out.
            _sendTicker.detach();
        }
};

#endif