#ifndef DASHBOARDSLAVECONF_H
#define DASHBOARDSLAVECONF_H

#include "carpi.h"

#ifdef NEW_SMALL
    #include "hardware/Pins_Dashboard_NEW_PCB.h"
#else
    #include "hardware/Pins_Dashboard_PCB.h"
#endif

// Communication
Sync syncer(DEVICE_DASHBOARD);
CCan canIntern(syncer, DASHBOARD_CAN);

// LED's
HardwareLed ledRed(DASHBOARD_PIN_LED_RED, COMPONENT_LED_ERROR);
HardwareLed ledYellow(DASHBOARD_PIN_LED_YELLOW, COMPONENT_LED_ISSUE);
HardwareLed ledGreen(DASHBOARD_PIN_LED_GREEN, COMPONENT_LED_READY_TO_DRIVE);


// Buttons
HardwareInterruptButton buttonReset(DASHBOARD_PIN_BUTTON_RESET, COMPONENT_BUTTON_RESET);
HardwareInterruptButton buttonStart(DASHBOARD_PIN_BUTTON_START, COMPONENT_BUTTON_START);

// Alive
HardwareAlive alive(COMPONENT_ALIVE_DASHBOARD, LED2);

class Dashboard : public Carpi {
    public:
        // Called once at bootup
        void setup() {
            syncer.addComponent((ICommunication&)ledRed, canIntern, DEVICE_MASTER);
            syncer.addComponent((ICommunication&)ledYellow, canIntern, DEVICE_MASTER);
            syncer.addComponent((ICommunication&)ledGreen, canIntern, DEVICE_MASTER);
            syncer.addComponent((ICommunication&)buttonReset, canIntern, DEVICE_MASTER);
            syncer.addComponent((ICommunication&)buttonStart, canIntern, DEVICE_MASTER);
            syncer.addComponent((ICommunication&)alive, canIntern, DEVICE_MASTER);
            syncer.finalize();

            wait(STARTUP_WAIT_TIME_SLAVE);

            // Attach the Syncer to all components
            ledRed.attach(syncer);
            ledYellow.attach(syncer);
            ledGreen.attach(syncer);
            buttonReset.attach(syncer);
            buttonStart.attach(syncer);
            alive.attach(syncer);

            alive.setAlive(true);
        }

        // Called repeately after bootup
        void loop() {
            syncer.run();
            wait(LOOP_WAIT_TIME);
        }
};

Dashboard runtime;

#endif