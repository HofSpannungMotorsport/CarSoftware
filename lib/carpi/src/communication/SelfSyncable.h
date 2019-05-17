#ifndef SELF_SYNCABLE_H
#define SELF_SYNCABLE_H

#include "Sync.h"
#include "components/interface/ICommunication.h"

class SelfSyncable : public ICommunication {
    public:
        virtual void attach(Sync &syncer) {
            _syncer = &syncer;
            _syncerAttached = true;
        }

    protected:
        Sync *_syncer;
        bool _syncerAttached = false;

        virtual void _getSubMessageCommand(car_sub_message_t &subMessage, uint8_t command, uint8_t value) {
            subMessage.length = 2;
            subMessage.data[0] = command;
            subMessage.data[1] = value;
        }

        virtual void _getSubMessageCommand(car_sub_message_t &subMessage, uint8_t command, uint8_t value, uint8_t value2) {
            _getSubMessageCommand(subMessage, command, value);
            subMessage.length = 3;
            subMessage.data[2] = value2;
        }

        virtual void _getSubMessageCommand(car_sub_message_t &subMessage, uint8_t command, uint8_t value, uint8_t value2, uint8_t value3, uint8_t value4) {
            _getSubMessageCommand(subMessage, command, value, value2);
            subMessage.length = 5;
            subMessage.data[3] = value3;
            subMessage.data[4] = value4;
        }

        virtual void _sendCommand(uint8_t command, uint8_t value, uint8_t priority, car_message_dropable_t dropable) {
            CarMessage carMessage;

            car_sub_message_t subMessage;
            _getSubMessageCommand(subMessage, command, value);
            carMessage.addSubMessage(subMessage);

            _send(carMessage, priority, dropable);
        }

        virtual void _sendCommand(uint8_t command, uint8_t value, uint8_t value2, uint8_t priority, car_message_dropable_t dropable) {
            CarMessage carMessage;

            car_sub_message_t subMessage;
            _getSubMessageCommand(subMessage, command, value, value2);
            carMessage.addSubMessage(subMessage);

            _send(carMessage, priority, dropable);
        }

        virtual void _sendCommand(uint8_t command, uint8_t value, uint8_t value2, uint8_t value3, uint8_t value4, uint8_t priority, car_message_dropable_t dropable) {
            CarMessage carMessage;

            car_sub_message_t subMessage;
            _getSubMessageCommand(subMessage, command, value, value2, value3, value4);
            carMessage.addSubMessage(subMessage);

            _send(carMessage, priority, dropable);
        }

        virtual void _send(CarMessage &carMessage, uint8_t priority, car_message_dropable_t dropable) {
            carMessage.setSendPriority(priority);
            carMessage.setComponentId(getComponentId());
            carMessage.setDropable(dropable);

            if (_syncerAttached)
                _syncer->send(carMessage);
        }
};

#endif // SELF_SYNCABLE_H