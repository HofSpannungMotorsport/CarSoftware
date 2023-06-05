#ifndef IDISPLAY_H
#define IDISPLAY_H

#include "ICommunication.h"

typedef uint8_t display_status_t;

class IDisplay : public ICommunication
{
public:
    virtual display_status_t getStatus() = 0;

    virtual void setBatteryVoltage(float) = 0;
    virtual void setSpeed(float) = 0;
    virtual void setAirTemperature(float) = 0;
    virtual void setMotorTemperature(float) = 0;
    virtual void setCurrent(uint16_t) = 0;
    virtual void setShutdownError(uint8_t) = 0;
    virtual void setPowermode(uint8_t) = 0;
    virtual void setGas(float) = 0;
    virtual void setBrake(float) = 0;
    virtual void setState(uint8_t) = 0;
};

#endif // IDISPLAY_H