#ifndef DASHBOARDSLAVECONF_H
#define DASHBOARDSLAVECONF_H

#define SYNC_USE_STACK_VECTOR
#define SYNC_MAX_DEVICES_COUNT 2
#define SYNC_MAX_CHANNELS_COUNT 1
#define SYNC_MAX_COMPONENTS_COUNT 8

#include "carpi.h"

#include "hardware/Pins_Dashboard_NEW_PCB.h"

// Communication
Sync syncer(DEVICE_DASHBOARD);
CCan canIntern(syncer, DASHBOARD_CAN);

// Registry
InternalRegistry registry(COMPONENT_SYSTEM_REGISTRY);

// LED's
HardwareLed ledRed(DASHBOARD_PIN_LED_RED, COMPONENT_LED_ERROR, registry);
HardwareLed ledYellow(DASHBOARD_PIN_LED_YELLOW, COMPONENT_LED_ISSUE, registry);
HardwareLed ledGreen(DASHBOARD_PIN_LED_GREEN, COMPONENT_LED_READY_TO_DRIVE, registry);
HardwareLed ledCI(DASHBOARD_PIN_LED_EXTRA, COMPONENT_LED_CI, registry);


// Buttons
HardwareInterruptButton buttonReset(DASHBOARD_PIN_BUTTON_RESET, COMPONENT_BUTTON_RESET, registry);
HardwareInterruptButton buttonStart(DASHBOARD_PIN_BUTTON_START, COMPONENT_BUTTON_START, registry);

// Alive
HardwareAlive alive(COMPONENT_ALIVE_DASHBOARD, DASHBOARD_PIN_ALIVE, registry);

class Dashboard : public Carpi {
    public:
        // Called once at bootup
        void setup() {
            wait(STARTUP_WAIT_TIME_SLAVE);

            syncer.addDevice(canIntern, DEVICE_MASTER);
            syncer.addDevice(canIntern, DEVICE_PEDAL);

            // Get Registry-Data from Master
            syncer.addComponent(registry, DEVICE_ALL);
            registry.attach(syncer);

            while (!registry.getReady()) {
                syncer.run();
                wait(LOOP_WAIT_TIME);
            }


            syncer.addComponent(ledRed, DEVICE_MASTER);
            syncer.addComponent(ledYellow, DEVICE_MASTER);
            syncer.addComponent(ledGreen, DEVICE_MASTER);
            syncer.addComponent(ledCI, DEVICE_MASTER);
            syncer.addComponent(buttonReset, DEVICE_MASTER);
            syncer.addComponent(buttonStart, DEVICE_MASTER);
            syncer.addComponent(alive, DEVICE_MASTER);

            #ifndef SYNC_USE_STACK_VECTOR
            syncer.finalize();
            #endif

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