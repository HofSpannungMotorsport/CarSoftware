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

        virtual void detach() {
            _syncerAttached = false;
        }

    protected:
        Sync *_syncer;
        bool _syncerAttached = false;

        virtual void _sendCommand(uint8_t command, car_message_dropable_t dropable) {
            CarMessage carMessage;

            carMessage.setLength(1);
            carMessage[0] = command;

            _send(carMessage, dropable);
        }

        virtual void _sendCommand(uint8_t command, uint8_t value, car_message_dropable_t dropable) {
            CarMessage carMessage;

            carMessage.setLength(2);
            carMessage[0] = command;
            carMessage[1] = value;

            _send(carMessage, dropable);
        }

        virtual void _sendCommand(uint8_t command, uint8_t values[], uint8_t valueCount, car_message_dropable_t dropable) {
            if (valueCount > 6) return; // carMessage maximum == 7 (-1 because of the command)

            CarMessage carMessage;

            carMessage.setLength(valueCount + 1); // +1 for the command
            carMessage[0] = command;

            memCpy<uint8_t>(&carMessage[1], values, valueCount);

            _send(carMessage, dropable);
        }

        virtual void _send(CarMessage &carMessage, car_message_dropable_t dropable) {
            carMessage.setComponentId(getComponentId());
            carMessage.setDropable(dropable);

            if (_syncerAttached)
                _syncer->send(carMessage);
        }

        uint32_t _convertFloat(float floatToConvert) {
            uint32_t convertedFloat = *((uint32_t*)&floatToConvert);
            return convertedFloat;
        }

        float _reconvertFloat(uint32_t floatToReconvert) {
            float reconvertedFloat = *((float*)&floatToReconvert);
            return reconvertedFloat;
        }

        void _disassambleFloat(float floatToDisassamble, uint8_t floatBinaryDisassambled[4]) {
            uint32_t floatBinary = _convertFloat(floatToDisassamble);

            for (uint8_t i = 0; i < 4; i++) {
                floatBinaryDisassambled[i] = (floatBinary >> (8 * i)) & 0xFF;
            }
        }

        float _reassambleFloat(uint8_t floatToReassamble[4]) {
            uint32_t floatBinary = 0;

            for (uint8_t i = 0; i < 4; i++) {
                floatBinary |= (((uint32_t)floatToReassamble[i] & 0xFF) << (i * 8));
            }

            return _reconvertFloat(floatBinary);
        }
};

#endif // SELF_SYNCABLE_H