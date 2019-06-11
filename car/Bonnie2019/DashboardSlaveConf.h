#ifndef DASHBOARDSLAVECONF_H
#define DASHBOARDSLAVECONF_H

#include "carpi.h"

#ifdef DASHBOARD_NEW
    #include "hardware/Pins_Dashboard_NEW_PCB.h"
#else
    #include "hardware/Pins_Dashboard_PCB.h"
#endif


CANService canService(DASHBOARD_CAN);

//LED's
HardwareLed ledRed(DASHBOARD_PIN_LED_RED, COMPONENT_LED_ERROR);
HardwareLed ledYellow(DASHBOARD_PIN_LED_YELLOW, COMPONENT_LED_ISSUE);
HardwareLed ledGreen(DASHBOARD_PIN_LED_GREEN, COMPONENT_LED_READY_TO_DRIVE);
HardwareLed ledCI(DASHBOARD_PIN_LED_EXTRA, COMPONENT_LED_CI);


// Buttons
HardwareInterruptButton buttonReset(DASHBOARD_PIN_BUTTON_RESET, COMPONENT_BUTTON_RESET);
HardwareInterruptButton buttonStart(DASHBOARD_PIN_BUTTON_START, COMPONENT_BUTTON_START);

class Dashboard : public Carpi {
    public:
        // Called once at bootup
        void setup() {
            canService.setSenderId(DEVICE_DASHBOARD);

            canService.addComponent((ICommunication*)&ledRed);
            canService.addComponent((ICommunication*)&ledYellow);
            canService.addComponent((ICommunication*)&ledGreen);
            canService.addComponent((ICommunication*)&ledCI);
            canService.addComponent((ICommunication*)&buttonReset);
            canService.addComponent((ICommunication*)&buttonStart);
        }

        // Called repeately after bootup
        void loop() {
            canService.run();

            if (buttonReset.getStateChanged()) {
                canService.sendMessage((ICommunication*)&buttonReset, DEVICE_MASTER);
            }

            if (buttonStart.getStateChanged()) {
                canService.sendMessage((ICommunication*)&buttonStart, DEVICE_MASTER);
            }
        }
};

Dashboard runtime;

#endif