#ifndef IBATTERY_H
#define IBATTERY_H

#include "ICommunication.h"

typedef float battery_voltage_t;

typedef uint8_t battery_status_t;
enum battery_error_type_t : uint8_t {
    VOLTAGE_TOO_HIGH =  0x1,
    VOLTAGE_TOO_LOW =   0x2,
};

class IBattery : public ICommunication {
    public:
        // setters
        //virtual void setVoltage(battery_voltage_t voltage) = 0;

        // getters
        virtual battery_status_t getStatus() = 0;
        virtual battery_voltage_t getVoltage() = 0;
};

#endif