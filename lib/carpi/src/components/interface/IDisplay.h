#ifndef IDISPLAY_H
#define IDISPLAY_H

#include "ICommunication.h"

typedef uint8_t display_status_t;

class IDisplay : public ICommunication
{
public:
    virtual display_status_t getStatus() = 0;

    virtual void setBatteryVoltage(float) = 0;
    virtual void setCellVoltage(float) = 0;
    virtual void setCellTemperatureMin(float) = 0;
    virtual void setCellTemperatureMax(float) = 0;
    virtual void setSpeed(float) = 0;
    virtual void setAirTemperature(float) = 0;
    virtual void setMotorTemperature(float) = 0;
    virtual void setCurrent(float) = 0;
    virtual void setPower(uint32_t) = 0;
    virtual void setShutdownError(uint8_t) = 0;
};

#endif // IDISPLAY_H