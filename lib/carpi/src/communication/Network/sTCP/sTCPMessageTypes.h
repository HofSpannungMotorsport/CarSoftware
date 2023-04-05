#pragma once

#include <stdint.h>

enum stcp_message_type_t : uint8_t {
    STCP_MESSAGE_TYPE_UNKNOWN = 0,
    STCP_MESSAGE_TYPE_OK = 1,
    STCP_MESSAGE_TYPE_RESTART = 2,
    STCP_MESSAGE_TYPE_RESET = 3,
    STCP_MESSAGE_TYPE_RESET_OK = 4,
};
