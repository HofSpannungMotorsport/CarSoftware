#ifndef DISPLAYSLAVECONF_H
#define DISPLAYSLAVECONF_H

#include "carpi.h"
#include "hardware/Pins_Display.h"

CANService canService(CAN_RX_PIN, CAN_TX_PIN);

// Displays
HardwareDisplay display(COMPONENT_DISPLAY_MAIN);

class Display
{
public:
    // Called once at bootup
    void setup()
    {
        pcSerial.printf("Initializing CAN\n");
        canService.setSenderId(DEVICE_DISPLAY);
        canService.addComponent((ICommunication *)&display);
    }
    void loop()
    {
        canService.run();
        display.lv_task();
        wait_ms(10);
    }
};

Display runtime;

#endif