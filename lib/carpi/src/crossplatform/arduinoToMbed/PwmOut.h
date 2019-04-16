#ifndef PWM_OUT_H
#define PWM_OUT_H

#include "AnalogOut.h"

class PwmOut : public AnalogOut {
    public:
        PwmOut(uint8_t pin) : AnalogOut(pin) {}
};

#endif // PWM_OUT_H