#ifndef SEND_PRIORITY_H
#define SEND_PRIORITY_H

#include "CarMessage.h"

enum send_priority_t : car_message_priority_t {
    SEND_PRIORITY_PEDAL = 0x1,
    SEND_PRIORITY_BUTTON = 0x2,
    SEND_PRIORITY_ALIVE = 0x3,
    SEND_PRIORITY_RPM = 0x4,
    SEND_PRIORITY_LED = 0x5
};

#endif // SEND_PRIORITY_H