#ifndef BUTTONMESSAGEHANDLER_H
#define BUTTONMESSAGEHANDLER_H

#include "mbed.h"
#include "IMessageHandler.h"
#include "../components/interface/IButton.h"

#define CAN_BUTTON_PAYLOAD_LEN 2

class ButtonMessageHandler : public IMessageHandler<CANMessage> {
    public:
        virtual msg_build_result_t buildMessage(void *objV, CANMessage &msg) {
            IButton *obj = (IButton*)objV;
            msg.len = CAN_BUTTON_PAYLOAD_LEN;

            msg.data[0] = obj->getState();
            msg.data[1] = obj->getStatus();

            return MSG_BUILD_OK;
        }

        virtual msg_parse_result_t parseMessage(void *objV, CANMessage &msg) {
            IButton *obj = (IButton*)objV;

            if(msg.len != CAN_BUTTON_PAYLOAD_LEN) // Not a valid message
                return MSG_PARSE_ERROR;
            
            obj->setState((button_state_t)msg.data[0]);
            obj->setStatus((button_status_t)msg.data[1]);

            return MSG_PARSE_OK;
        }
};

#endif // BUTTONMESSAGEHANDLER_H