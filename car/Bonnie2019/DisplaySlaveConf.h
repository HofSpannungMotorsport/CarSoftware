#ifndef DISPLAYSLAVECONF_H
#define DISPLAYSLAVECONF_H

#include "carpi.h"

#include "hardware/Pins_Display.h"

// Communication
Sync syncer(DEVICE_DISPLAY);
CCan canIntern(syncer, 0, 0); // Underlying FlexCAN Library using standard Teensy CAN Pins -> 0

class Display : public Carpi {
    public:
        void setup() {
            syncer.finalize();

            wait(STARTUP_WAIT_TIME_SLAVE);

        }

        void loop() {
            syncer.run();
            wait(LOOP_WAIT_TIME);
        }

    protected:

};

Display runtime;

#endif // DISPLAYSLAVECONF_H