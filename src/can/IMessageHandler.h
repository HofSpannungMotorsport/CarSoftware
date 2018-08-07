#ifndef IMESSAGE_HANDLER_H
#define IMESSAGE_HANDLER_H

#include "mbed.h"

enum msg_build_result_t
{
    OK,
    ERROR
};

enum msg_parse_result_t
{
    OK,
    ERROR
};

template<class T>
class IMessageHandler
{
    public:
    virtual msg_build_result_t buildMessage(T &obj, CANMessage &msg) = 0;
    virtual msg_parse_result_t parseMessage(T &obj, CANMessage &msg) = 0;
};

#endif