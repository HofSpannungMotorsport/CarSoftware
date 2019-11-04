#ifndef ANALOG_IN_H
#define ANALOG_IN_H

#include "../NonCopyable.h"

#define ANALOG_IN_RESOLUTION 10 // bit

class AnalogIn : private NonCopyable<AnalogIn> {
    public:
        AnalogIn(uint8_t pin)
        : _pin(pin) {
            pinMode(pin, INPUT);
            _devider = uint16_t_pow(2, ANALOG_IN_RESOLUTION);
        }

        float read() {
            return (float)analogRead(_pin) / (float)_devider;
        }

        uint16_t read_u16() {
            return analogRead(_pin) << ANALOG_IN_RESOLUTION;
        }

        operator float() {
            return read();
        }

    private:
        uint8_t _pin;
        uint16_t _devider;
};

#endif // ANALOG_IN_H