#ifndef SEND_PRIORITY_H
#define SEND_PRIORITY_H

#include "CarMessage.h"

enum send_priority_t : car_message_priority_t {
    SEND_PRIORITY_ALIVE = 0x0,
    SEND_PRIORITY_BUTTON,
    SEND_PRIORITY_LED,
    SEND_PRIORITY_PEDAL,
    SEND_PRIORITY_RPM

};

#endif // SEND_PRIORITY_H