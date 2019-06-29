#ifndef ICOMMUNICATION_H
#define ICOMMUNICATION_H

#include "communication/CarMessage.h"
#include "IComponent.h"

enum message_build_result_t : uint8_t {
    MESSAGE_BUILD_OK = 0x1,
    MESSAGE_BUILD_ERROR = 0x2
};

enum message_parse_result_t : uint8_t {
    MESSAGE_PARSE_OK = 0x1,
    MESSAGE_PARSE_ERROR = 0x2
};

class ICommunication : public IComponent {
    public:
        virtual message_build_result_t buildMessage(CarMessage &carMessage) = 0;
        virtual message_parse_result_t parseMessage(CarMessage &carMessage) = 0;
};

#endif // ICOMMUNICATION_H