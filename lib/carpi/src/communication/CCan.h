#ifndef CCAN_H
#define CCAN_H

#include <vector>
using namespace std;

#include "Sync.h"

#ifndef DEVICE_CAN
    #define DEVICE_CAN // Otherwise a compiler issue (not error) -> defined by mbed before?
#endif

#define STD_CAN_FREQUENCY 250000

#define STD_CAN_TIMEOUT (1.0/(float)STD_CAN_FREQUENCY) // s

class CCan : public IChannel {
    public:
        CCan() = delete;
        CCan(Sync &syncer, PinName rx, PinName tx, int frequency = STD_CAN_FREQUENCY)
        : _syncer(syncer), _can(rx, tx, frequency) {
            _can.attach(callback(this, &CCan::_canMessageReceive), CAN::RxIrq);
        }

        void send(CarMessage &carMessage) {
            _outQueue.push_back(carMessage);
            _send();
        }
    
    private:
        CAN _can;
        Sync &_syncer;

        float _canTimeout = STD_CAN_TIMEOUT;

        vector<CarMessage> _outQueue;
        vector<CarMessage> _outQueueTimeout;

        Ticker _sendTicker;

        void _canMessageReceive() {
            CANMessage canMessage = CANMessage();

            while(_can.read(canMessage)) {
                CarMessage carMessage;
                _getCarMessage(canMessage, carMessage);
                _syncer.receive(carMessage);
            }
        }

        void _getCarMessage(CANMessage &canMessage, CarMessage &carMessage) {
            car_sub_message_t subMessage;

            carMessage.setIdsFromMessageHeader(canMessage.id);
            carMessage.setComponentId(canMessage.data[0]);

            for (uint8_t i = 0; i < canMessage.len-1; i++) {
                subMessage.data[i] = canMessage.data[i+1];
            }

            carMessage.addSubMessage(subMessage);
        }

        void _getCanMessage(CarMessage &carMessage, uint16_t subMessageNumber, CANMessage canMessage) {
            car_sub_message_t &subMessage = carMessage.subMessages[subMessageNumber];

            canMessage.format = CANStandard;
            canMessage.id = carMessage.getMessageHeader();
            canMessage.len = subMessage.length + 1;
            canMessage.data[0] = carMessage.getComponentId();

            for (uint8_t i = 0; i < subMessage.length; i++) {
                canMessage.data[i+1] = subMessage.data[i];
            }
        }

        car_message_priority_t _getHighestPriortiy(vector<CarMessage> &queue) {
            car_message_priority_t highestPriorityFound = CAR_MESSAGE_PRIORITY_HIGHEST;
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
                            
                            // Now try to send the Message over CAN
                            int canWriteResult = _can.write(canMessage);
                            if (canWriteResult == 1) {
                                // If sent successfully the first subMessage, remove it and look if there are more
                                carMessageIterator->removeFirstSubMessage();
                                if(carMessageIterator->subMessages.empty()) {
                                    // If it was the last/only one, remove the whole Message
                                    carMessageIterator = _outQueueTimeout.erase(carMessageIterator);
                                }
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
                            
                            // Now try to send the Message over CAN
                            int canWriteResult = _can.write(canMessage);
                            if (canWriteResult == 1) {
                                // If sent successfully the first subMessage, remove it and look if there are more
                                carMessageIterator->removeFirstSubMessage();
                                if(carMessageIterator->subMessages.empty()) {
                                    // If it was the last/only one, remove the whole Message
                                    carMessageIterator = _outQueue.erase(carMessageIterator);
                                }
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