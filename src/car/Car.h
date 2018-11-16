#ifndef CAR_H

#define MESSAGE_REPORT
#ifdef MESSAGE_REPORT
    #include "mbed.h"
    Serial pcSerial(USBTX, USBRX); // Connection to PC over Serial
#endif

#ifdef CAR_BONNIE_2019
#include "Bonnie2019/Bonnie2019.h"
#endif

#endif