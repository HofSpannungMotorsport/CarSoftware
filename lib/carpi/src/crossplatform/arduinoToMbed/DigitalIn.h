#ifndef DIGITAL_IN_H
#define DIGITAL_IN_H

#include "../NonCopyable.h"

class DigitalIn : private NonCopyable<DigitalIn> {
    public:
        DigitalIn(uint8_t pin)
        : _pin(pin) {
            pinMode(pin, INPUT);
        }

        uint8_t read() {
            if (digitalRead(_pin) == HIGH) {
                return 1;
            } else {
                return 0;
            }
        }

        operator uint8_t() {
            return read();
        }
    
    private:
        uint8_t _pin;
};

#endif