#ifndef IBATTERY_H
#define IBATTERY_H

#include "ICommunication.h"

typedef float cell_voltage_t;
typedef float cell_temperature_t;
typedef uint8_t cell_status_t;

enum battery_error_type_t : uint8_t {
    VOLTAGE_TOO_HIGH    =   0x1,
    VOLTAGE_TOO_LOW     =   0x2,
    TEMPERATURE_TOO_LOW =   0x4,
    TEMPERATURE_TOO_LOW =   0x8,
};

class ICell : public ICommunication {
    public:
        // setters
        //virtual void setVoltage(cell_voltage_t voltage) = 0;
        //virtual void setTemperature(cell_temperature_t temperature) = 0;

        // getters
        virtual cell_status_t getStatus() = 0;
        virtual cell_voltage_t getVoltage() = 0;
        virtual cell_temperature_t getTemperature() = 0;
};

#endif