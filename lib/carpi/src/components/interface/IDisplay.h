#ifndef IDISPLAY_H
#define IDISPLAY_H

#include "ICommunication.h"

typedef uint8_t display_status_t;

class IDisplay : public ICommunication {
    public:

        virtual display_status_t getStatus() = 0;

        virtual uint16_t getMinCellVoltage() = 0;

        virtual uint16_t getMaxCellVoltage() = 0;

        virtual uint16_t getMinCellTemperature() = 0;

        virtual uint16_t getMaxCellTemperature() = 0;

        

        virtual uint16_t setMinCellVoltage(uint16_t value) = 0;
        virtual uint16_t setMaxCellVoltage(uint16_t value) = 0;
        virtual uint16_t setMinCellTemperature(uint16_t value) = 0;
        virtual uint16_t setMaxCellTemperature(uint16_t value) = 0;

        virtual float setSpeed(float value) = 0;
        virtual float setCurrent(float value) = 0;

        virtual uint16_t setMotorTemp(uint16_t value) = 0;

        virtual uint16_t setAirTemp(uint16_t value) = 0;

        virtual float setDcVoltage(float value) = 0;

};

#endif