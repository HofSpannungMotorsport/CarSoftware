#ifndef DISPLAYSLAVECONF_H
#define DISPLAYSLAVECONF_H

// #define REPORT_CAN_ERROR

#include "carpi.h"

#include "hardware/Pins_Display.h"

#define SEND_RATE 2 // hz

SoftwareBattery batteryVoltage(COMPONENT_BATTERY_VOLTAGE);
CANService canService(DISPLAY_CAN);

class Display : public Carpi {
    public:
        // Called once at bootup
        void setup() {
            canService.setSenderId(DEVICE_DISPLAY);
            canService.addComponent((ICommunication*)&batteryVoltage);
        }

        // Called repeately after bootup
        void loop() {
            canService.run();
            wait(1.0/(float) SEND_RATE);
            
        }
};

Display runtime;

#endif
