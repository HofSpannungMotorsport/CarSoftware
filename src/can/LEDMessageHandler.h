#ifndef LEDMESSAGE_HANDLER_H
#define LEDMESSAGE_HANDLER_H

#include "IMessageHandler.h"
#include "ILed.h"
#include "mbed.h"

#define CAN_LED_PAYLOAD_LEN 1

class LEDMessageHandler : public IMessageHandler<CANMessage>
{
    public:
    virtual msg_build_result_t buildMessage(void *objV, CANMessage &msg)
    {
        ILed *obj = (ILed*)objV;
        msg.data[0] = ((obj->getState() & 0x1u) << 7) | ((obj->getBrightness() & 0x2u) << 5) | ((obj->getBlinking() & 0x2u) << 3);
        msg.len = CAN_LED_PAYLOAD_LEN;
        return MSG_BUILD_OK;
    }
    virtual msg_parse_result_t parseMessage(void *objV, CANMessage &msg)
    {
        ILed *obj = (ILed*)objV;

        if(msg.len != CAN_LED_PAYLOAD_LEN) // not a valid message for leds
            return MSG_PARSE_ERROR;
        
        obj->setState((led_state_t)((msg.data[0] & 0x80u) >> 7));
        obj->setBrightness((msg.data[0] & 0x60u) >> 5);
        obj->setBlinking((led_blinking_t)((msg.data[0] & 0x18u) >> 3));

        return MSG_PARSE_OK;
    }
};

#endif