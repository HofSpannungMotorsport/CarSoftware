#ifndef LEDMESSAGE_HANDLER_H
#define LEDMESSAGE_HANDLER_H

#include "IMessageHandler.h"
#include "ILed.h"
#include "mbed.h"

class LEDMessageHandler : public IMessageHandler<ILed>
{
    public:
    virtual msg_build_result_t buildMessage(ILed &obj, CANMessage &msg)
    {
        return MSG_BUILD_ERROR;
    }
    virtual msg_parse_result_t parseMessage(ILed &obj, CANMessage &msg)
    {
        if(msg.len != 1) // not a valid message for leds
            return MSG_PARSE_ERROR;
        
        obj.setState((led_state_t)((msg.data[0] & 0x80u) >> 7));
        obj.setBrightness((msg.data[0] & 0x60u) >> 5);
        obj.setBlinking((led_blinking_t)((msg.data[0] & 0x18u) >> 3));

        return MSG_PARSE_OK;
    }
};

#endif