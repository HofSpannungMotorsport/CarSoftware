#ifndef DISPLAYSLAVECONF_H
#define DISPLAYSLAVECONF_H

// #define REPORT_CAN_ERROR

#include "carpi.h"

#include "hardware/Pins_Display.h"

#define SEND_RATE 5 // hz

SoftwareMotorController motorController(COMPONENT_MOTOR_MAIN);
CANService canService(DISPLAY_CAN_RD, DISPLAY_CAN_TD);

class Display : public Carpi
{
public:
    // Called once at bootup
    void setup()
    {
        canService.setSenderId(DEVICE_DISPLAY);
        canService.addComponent((ICommunication *)&motorController);
    }

    // Called repeately after bootup
    void loop()
    {
        canService.run();
        wait(1.0 / (float)SEND_RATE);
    }
};

Display runtime;

#endif
