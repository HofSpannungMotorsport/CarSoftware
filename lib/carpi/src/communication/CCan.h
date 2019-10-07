#ifndef CCAN_H
#define CCAN_H

#ifdef USE_ARDUINO
    #error "CCan does only support mbed and Teensyduino, not Arduino!"
#endif

#include <vector>
using namespace std;

#include "Sync.h"

#ifndef DEVICE_CAN
    #define DEVICE_CAN // Otherwise a compiler issue (not error) -> defined by mbed before?
#endif

#define STD_CAN_FREQUENCY 250000

#define STD_MAX_OUT_QUEUE_SIZE 128 // Elements
#define STD_OUT_QUEUE_IMPORTANT_THRESHHOLD STD_MAX_OUT_QUEUE_SIZE/2

#define STD_CAN_TIMEOUT (1.0/(float)STD_CAN_FREQUENCY) // s

class CCan : public IChannel {
    public:
        CCan() = delete;
        CCan(Sync &syncer, PinName rx, PinName tx, int frequency = STD_CAN_FREQUENCY)
        : _syncer(syncer), _can(rx, tx, frequency) {
            #ifdef USE_MBED
                _can.attach(callback(this, &CCan::_canMessageReceive), CAN::RxIrq);
            #endif
        }

        virtual void send(CarMessage &carMessage) {
            // Drop oldest message if _dropQueue is full
            while (_outQueue.size() >= _maxSize.outQueue) {
                _dropOldestMessage(_outQueue);
                _dropped.outQueue++;
            }

            _outQueue.push_back(carMessage);
            _send();
        }

        virtual bool messageInQueue() {
            return !_outQueue.empty();
        }

        virtual void run() {
            #ifdef USE_TEENSYDUINO
                if (_can.available() > 0) {
                    _canMessageReceive();
                }
            #endif

            _send();

            #if defined(MESSAGE_REPORT) && defined(USE_MBED)
                if (_can.tderror() > 0 || _can.rderror() > 0) {
                    pcSerial.printf("[CCan]@run: Detected CAN Error: td: %i\t rd: %i\n", _can.tderror(), _can.rderror());
                    _can.reset();
                }
            #endif
        }
    
    private:
        CAN _can;
        Sync &_syncer;

        vector<CarMessage> _outQueue;

        struct _dropped {
            uint32_t outQueue = 0;
        } _dropped;

        struct _maxSize {
            uint16_t outQueue = STD_MAX_OUT_QUEUE_SIZE;
        } _maxSize;

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

            #if defined(CCAN_DEBUG) && defined(MESSAGE_REPORT)
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

            #if defined(CCAN_SENDING_DEBUG) && defined(MESSAGE_REPORT)
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
            // Check if there are Messages to send and try to send them if possible
            while(!_outQueue.empty()) {
                // Search highest priority in two steps
                // First find the highest priority (the smallest priority-number)
                // then find the first message with this priortiy and send it

                car_message_priority_t highestPriortiy = _getHighestPriortiy(_outQueue);

                for(auto carMessageIterator = _outQueue.begin(); carMessageIterator != _outQueue.end(); ) {
                    if (carMessageIterator->getSendPriority() == highestPriortiy) {
                        CANMessage canMessage;
                        _getCanMessage(*carMessageIterator, 0, canMessage);

                        if (_outQueue.size() >= STD_OUT_QUEUE_IMPORTANT_THRESHHOLD) {
                            // Message has to be more important to clear the _outQueue more fast
                            // By default, the first bit of the message id is 0 and so, it is more important.
                            // -> Nothing to do
                        } else {
                            // But if the _outQueue is small, the message is made less important by setting the first
                            // id bit to 1
                            canMessage.id |= 0x400; // -> Binary 10000000000
                        }
                        

                        #if defined(CCAN_SENDING_DEBUG) && defined(MESSAGE_REPORT)
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

                            #if defined(CCAN_SENDING_DEBUG) && defined(MESSAGE_REPORT)
                                pcSerial.printf("[CCan]@_send->_outQueue: Successfully sent canMessage with component ID 0x%x and message ID 0x%x\n", canMessage.data[0], canMessage.id);
                            #endif
                        }
                    }
                }
            }
        }
};

#endif