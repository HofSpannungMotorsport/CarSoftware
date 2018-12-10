#ifndef PEDAL_MESSAGE_HANDLER_H
#define PEDAL_MESSAGE_HANDLER_H

#include "IMessageHandler.h"
#include "../components/interface/IPedal.h"
#include "mbed.h"

#define CAN_HARDWARE_PEDAL_PAYLOAD_LEN 3
#define CAN_SOFTWARE_PEDAL_PAYLOAD_LEN 1

class PedalMessageHandler : public IMessageHandler<CANMessage> {
    public:
        virtual msg_build_result_t buildMessage(void *objV, CANMessage &msg) {
            can_object_type_datatype_t objectType = (msg.id & 0x3);
            if (objectType == HARDWARE_OBJECT) { // If sending from an HARDWARE_OBJECT
                IPedal *obj = (IPedal*)objV;
                msg.len = CAN_HARDWARE_PEDAL_PAYLOAD_LEN;

                msg.data[0] = obj->getStatus();

                // change line below when type of pedal_value_t changes
                float pedalValueFloat = obj->getValue();
                uint16_t pedalValue = ((float)pedalValueFloat * 65535);

                #ifdef PEDAL_MESSAGE_HANDLER_DEBUG
                    pcSerial.printf("[PedalMessageHandler]@buildMessage: HardwareObject (float)pedalValue: %.3f\t(uint16_t)pedalValue: %i\t", pedalValueFloat, pedalValue);
                #endif

                msg.data[1] = pedalValue & 0xFF;
                msg.data[2] = (pedalValue >> 8) & 0xFF;

                #ifdef PEDAL_MESSAGE_HANDLER_DEBUG
                    pcSerial.printf("msg.data[1]: 0x%x\tmsg.data[2]: 0x%x\n", msg.data[1], msg.data[2]);
                #endif

                return MSG_BUILD_OK;
            } else if (objectType == SOFTWARE_OBJECT) {
                IPedal *obj = (IPedal*)objV;
                msg.len = CAN_SOFTWARE_PEDAL_PAYLOAD_LEN;

                pedal_calibration_t calibrationStatus = obj->getCalibrationStatus();
                uint8_t valueToSent = 0;

                if (calibrationStatus == CURRENTLY_NOT_CALIBRATING) {
                    valueToSent = 1;
                } else if (calibrationStatus == CURRENTLY_CALIBRATING) {
                    valueToSent = 2;
                }

                msg.data[0] = valueToSent;

                #ifdef PEDAL_MESSAGE_HANDLER_DEBUG
                    pcSerial.printf("[PedalMessageHandler]@buildMessage: SoftwareObject calibrationStatus: 0x%x\tmsg.data[0]: 0x%x\tvalueToSent: %i\n", calibrationStatus, msg.data[0], valueToSent);
                #endif

                return MSG_BUILD_OK;
            } else {
                return MSG_BUILD_ERROR;
            }
        }

        virtual msg_parse_result_t parseMessage(void *objV, CANMessage &msg) {
            can_object_type_datatype_t objectType = (msg.id & 0x3);
            if (objectType == HARDWARE_OBJECT) { // If receiving from and HARDWARE_OBJECT
                IPedal *obj = (IPedal*)objV;

                if(msg.len != CAN_HARDWARE_PEDAL_PAYLOAD_LEN) // not a valid message
                    return MSG_PARSE_ERROR;

                obj->setStatus(msg.data[0]);

                // change line below when type of pedal_value_t changes
                uint16_t newValue16 = msg.data[1] | (msg.data[2] << 8);
                float newValue = (float)newValue16 / 65535.0;

                obj->setValue(newValue);

                #ifdef PEDAL_MESSAGE_HANDLER_DEBUG
                    pcSerial.printf("[PedalMessageHandler]@parseMessage: HardwareObject (float)pedalValue: %.3f\t(uint16_t)pedalValue: %i\tmsg.data[1]: 0x%x\tmsg.data[2]: 0x%x\n", newValue, newValue16, msg.data[1], msg.data[2]);
                #endif

                return MSG_PARSE_OK;
            } else if (objectType == SOFTWARE_OBJECT) {
                IPedal *obj = (IPedal*)objV;

                if(msg.len != CAN_SOFTWARE_PEDAL_PAYLOAD_LEN) // not a valid message
                    return MSG_PARSE_ERROR;

                uint8_t gotValue = msg.data[0];
                pedal_calibration_t calibrationStatus = CURRENTLY_NOT_CALIBRATING;

                if (gotValue == 1) {
                    calibrationStatus = CURRENTLY_NOT_CALIBRATING;
                } else if (gotValue == 2) {
                    calibrationStatus = CURRENTLY_CALIBRATING;
                }

                obj->setCalibrationStatus(calibrationStatus);

                #ifdef PEDAL_MESSAGE_HANDLER_DEBUG
                    pcSerial.printf("[PedalMessageHandler]@parseMessage: SoftwareObject calibrationStatus: 0x%x\tmsg.data[0]: 0x%x\tgotValue: %i\n", calibrationStatus, msg.data[0], gotValue);
                #endif

                return MSG_PARSE_OK;
            } else {
                return MSG_PARSE_ERROR;
            }
        }
};

#endif // PEDAL_MESSAGE_HANDLER_H