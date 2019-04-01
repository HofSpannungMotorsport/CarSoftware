#ifndef ICOMMUNICATION_H
#define ICOMMUNICATION_H

#include "communication/CarMessage.h"
#include "IID.h"

enum message_build_result_t : uint8_t {
    MESSAGE_BUILD_OK = 0x1,
    MESSAGE_BUILD_ERROR = 0x2
};

enum message_parse_result_t : uint8_t {
    MESSAGE_PARSE_OK = 0x1,
    MESSAGE_PARSE_ERROR = 0x2
};

class ICommunication : public IID {
    public:
        virtual message_build_result_t buildMessage(CarMessage &carMessage);
        virtual message_parse_result_t parseMessage(CarMessage &carMessage);
};

#endif // ICOMMUNICATION_H