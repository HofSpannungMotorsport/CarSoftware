#ifndef ARDUINO_TO_MBED_H
#define ARDUINO_TO_MBED_H

uint16_t uint16_t_pow(uint16_t base, uint16_t exponent) {
    if (exponent == 0) return 1;

    for (uint16_t i = 1; i < exponent; i++) {
        base *= base;
    }

    return base;
}

#include "PinName.h"
#include "../NonCopyable.h"
#include "AnalogIn.h"
#include "AnalogOut.h"
#include "DigitalIn.h"
#include "DigitalOut.h"
#include "PwmOut.h"
#include "Timer.h"
#include "CircularBuffer.h"
#include "CAN.h"

void wait(float s) {
    Timer timer;
    timer.start();
    while(timer < s);
}

#endif // ARDUINO_TO_MBED_H