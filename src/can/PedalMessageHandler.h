#ifndef PEDAL_MESSAGE_HANDLER_H
#define PEDAL_MESSAGE_HANDLER_H

#include "IMessageHandler.h"
#include "../components/interface/IPedal.h"
#include "mbed.h"

#define CAN_HARDWARE_PEDAL_PAYLOAD_LEN 2
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
                msg.data[1] = (uint8_t)(obj->getValue() * 255.0);
                return MSG_BUILD_OK;
            } else if (objectType == SOFTWARE_OBJECT) {
                IPedal *obj = (IPedal*)objV;
                msg.data[0] = (uint8_t)obj->getCalibrationStatus();
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
                obj->setValue(msg.data[1]/255.0);

                return MSG_PARSE_OK;
            } else if (objectType == SOFTWARE_OBJECT) {
                IPedal *obj = (IPedal*)objV;

                if(msg.len != CAN_HARDWARE_PEDAL_PAYLOAD_LEN) // not a valid message
                    return MSG_PARSE_ERROR;

                obj->setCalibrationStatus((pedal_calibration_t)msg.data[0]);

                return MSG_PARSE_OK;
            } else {
                return MSG_PARSE_ERROR;
            }
        }
};

#endif // PEDAL_MESSAGE_HANDLER_H