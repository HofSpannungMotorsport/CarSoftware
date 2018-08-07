#ifndef IPEDAL_H
#define IPEDAL_H

#include <stdint.h>

typedef uint8_t pedal_status_t;
typedef uint8_t pedal_value_t;

class IPedal
{
    public:
        virtual pedal_status_t getStatus() = 0;
        virtual pedal_value_t getValue() = 0;
        virtual void setStatus(pedal_status_t status) = 0;
        virtual void setValue(pedal_value_t value) = 0;
};

#endif