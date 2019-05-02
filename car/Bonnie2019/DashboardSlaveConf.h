#ifndef DASHBOARDSLAVECONF_H
#define DASHBOARDSLAVECONF_H

#include "carpi.h"

#include "hardware/Pins_Dashboard_PCB.h"

// Communication
Sync syncer(DEVICE_DASHBOARD);
CCan canIntern(syncer, DASHBOARD_CAN);

//LED's
HardwareLed ledRed(DASHBOARD_PIN_LED_RED, COMPONENT_LED_ERROR);
HardwareLed ledYellow(DASHBOARD_PIN_LED_YELLOW, COMPONENT_LED_ISSUE);
HardwareLed ledGreen(DASHBOARD_PIN_LED_GREEN, COMPONENT_LED_READY_TO_DRIVE);


// Buttons
HardwareInterruptButton buttonReset(DASHBOARD_PIN_BUTTON_RESET, COMPONENT_BUTTON_RESET);
HardwareInterruptButton buttonStart(DASHBOARD_PIN_BUTTON_START, COMPONENT_BUTTON_START);

class Dashboard : public Carpi {
    public:
        // Called once at bootup
        void setup() {
            syncer.addComponent((ICommunication&)ledRed, canIntern, DEVICE_MASTER);
            syncer.addComponent((ICommunication&)ledYellow, canIntern, DEVICE_MASTER);
            syncer.addComponent((ICommunication&)ledGreen, canIntern, DEVICE_MASTER);
            syncer.addComponent((ICommunication&)buttonReset, canIntern, DEVICE_MASTER);
            syncer.addComponent((ICommunication&)buttonStart, canIntern, DEVICE_MASTER);
        }

        // Called repeately after bootup
        void loop() {
            wait(0.0001);
        }
};

Dashboard runtime;

#endif