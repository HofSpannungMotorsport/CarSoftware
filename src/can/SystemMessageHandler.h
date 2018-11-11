#ifndef SYSTEM_CALIBRATION_MESSAGE_HANDLER_H
#define SYSTEM_CALIBRATION_MESSAGE_HANDLER_H

#include "IMessageHandler.h"
#include "IPedal.h"
#include "mbed.h"

#define CAN_PEDAL_PAYLOAD_LEN 2

class PedalMessageHandler : public IMessageHandler<CANMassage> {
    public:
        virtual msg_build_result_t buildMessage(void *objV, CANMessage &msg) {
            IPedal *obj = (IPedal*)objV;
            msg.len = CAN_PEDAL_PAYLOAD_LEN;

            msg.data[0] = obj->getStatus();
        
            // change line below when type of pedal_value_t changes
            msg.data[1] = (uint8_t)(obj->getValue() * 255.0);
            return MSG_BUILD_OK;
        }

        virtual msg_parse_result_t parseMessage(void *objV, CANMessage &msg) {
            IPedal *obj = (IPedal*)objV;

            if(msg.len != CAN_PEDAL_PAYLOAD_LEN) // not a valid message
                return MSG_PARSE_ERROR;
            
            obj->setStatus(msg.data[0]);
            
            // change line below when type of pedal_value_t changes
            obj->setValue(msg.data[1]/255.0);

            return MSG_PARSE_OK;
        }
};

#endif // SYSTEM_CALIBRATION_MESSAGE_HANDLER_H