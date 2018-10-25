#ifndef IMESSAGE_HANDLER_H
#define IMESSAGE_HANDLER_H

#include "mbed.h"

enum msg_build_result_t {
    MSG_BUILD_OK,
    MSG_BUILD_ERROR
};

enum msg_parse_result_t {
    MSG_PARSE_OK,
    MSG_PARSE_ERROR
};

template<class T>
class IMessageHandler {
    public:
        virtual msg_build_result_t buildMessage(void *obj, T &msg) = 0;
        virtual msg_parse_result_t parseMessage(void *obj, T &msg) = 0;
};

#endif