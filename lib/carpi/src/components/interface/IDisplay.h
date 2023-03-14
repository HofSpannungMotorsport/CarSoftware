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
    virtual void setCellTemperature(float) = 0;
    virtual void setSpeed(float) = 0;
};

#endif // IDISPLAY_H