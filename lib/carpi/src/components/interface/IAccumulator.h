#ifndef IACCUMULATOR_H
#define IACCUMULATOR_H

#include "ICommunication.h"

typedef uint8_t accumulator_status_t;

enum accumulator_error_type_t : uint8_t {
    TEMPERATURE_ERROR   = 0x1,
    VOLTAGE_ERROR       = 0x2
};


class IAccumulator : public ICommunication {
    public:
    
        //Status
        virtual accumulator_status_t getStatus() = 0;

        //Cell-Voltage
        virtual uint16_t getMinVoltage() = 0;
        virtual uint16_t getMaxVoltage() = 0;
        
        //Cell-Temperature
        virtual uint16_t getMinTemp() = 0;
        virtual uint16_t getMaxTemp() = 0;

};
#endif // IACCUMULATOR_H