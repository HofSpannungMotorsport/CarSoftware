#ifndef DISPLAYSLAVECONF_H
#define DISPLAYSLAVECONF_H

#define EXPERIMENTELL_ASR_ACTIVE
// #define DEBUG_PRINT_RAW_ANALOG
#define REPORT_CAN_ERROR

// Automatic
#ifdef EXPERIMENTELL_ASR_ACTIVE
#define ENABLE_POWER_MENU
#endif

// #define CAN_DEBUG
#include "carpi.h"

#include "hardware/Pins_Display.h"

CANService canService(DISPLAY_CAN);

#define DISPLAY_SEND_RATE 120 // Hz

// Displays
HardwareDisplay display();

class Display : public Carpi
{
public:
    // Called once at bootup
    void setup()
    {
        canService.setSenderId(DEVICE_DISPLAY);

        canService.addComponent((ICommunication *)&display);
        wait(0.1);

#ifdef DEBUG_PRINT_RAW_ANALOG
        reportTimer.start();
#endif
    }

    // Called repeately after bootup
    void loop()
    {
        canService.run();

        wait(1.0 / (float)DISPLAY_SEND_RATE);
    }

#ifdef DEBUG_PRINT_RAW_ANALOG
private:
    Timer reportTimer;
#endif
};

Display runtime;

#endif