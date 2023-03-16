#ifndef DISPLAYSLAVECONF_H
#define DISPLAYSLAVECONF_H

#include "carpi.h"

#include "hardware/Pins_Display.h"

#define DISPLAY_SEND_RATE 10 // Hz

CANService canService(CAN_RX_PIN, CAN_TX_PIN);

// Displays
HardwareDisplay display();
// SoftwareMotorController motorcontroller();

class Display : public Carpi
{
public:
    // Called once at bootup
    void setup()
    {
        canService.setSenderId(DEVICE_DISPLAY);

        canService.addComponent((ICommunication *)&display);
        // canService.addComponent((ICommunication *)&motorcontroller);
        wait(0.1);
    }

    // Called repeately after bootup
    void loop()
    {
        canService.run();

        wait(1.0 / (float)DISPLAY_SEND_RATE);
    }
};

Display runtime;

#endif