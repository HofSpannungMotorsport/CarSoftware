#ifndef LEDMESSAGE_HANDLER_H
#define LEDMESSAGE_HANDLER_H

#include "IMessageHandler.h"
#include "../components/interface/ILed.h"
#include "mbed.h"

#define CAN_LED_PAYLOAD_LEN 1

class LEDMessageHandler : public IMessageHandler<CANMessage> {
    public:
        virtual msg_build_result_t buildMessage(void *objV, CANMessage &msg) {
            ILed *obj = (ILed*)objV;
            msg.data[0] = (obj->getState() & 0x1) << 7;
            msg.data[0] |= ((uint8_t)(obj->getBrightness() * 4) & 0x3) << 5;
            msg.data[0] |= ((obj->getBlinking() & 0x3) << 3);
            msg.len = CAN_LED_PAYLOAD_LEN;
            return MSG_BUILD_OK;
        }

        virtual msg_parse_result_t parseMessage(void *objV, CANMessage &msg) {
            ILed *obj = (ILed*)objV;

            if(msg.len != CAN_LED_PAYLOAD_LEN) // not a valid message for leds
                return MSG_PARSE_ERROR;
        
            obj->setState((led_state_t)((msg.data[0] >> 7) & 0x1));
            obj->setBrightness((float)((msg.data[0] >> 5) & 0x3) / 4);
            obj->setBlinking((led_blinking_t)((msg.data[0] >> 3) & 0x3));

            return MSG_PARSE_OK;
        }
};

#endif // LEDMESSAGE_HANDLER_H