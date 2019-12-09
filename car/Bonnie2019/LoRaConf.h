#ifndef LORACONF_H
#define LORACONF_H

#include "carpi.h"


// Communication
Sync syncer(DEVICE_LORA);

class LoRa : public Carpi {
    public:
        // Called once at bootup
        void setup() {
            wait(STARTUP_WAIT_TIME_SLAVE);
        }

        // Called repeately after bootup
        void loop() {
            syncer.run();
            wait(LOOP_WAIT_TIME);
        }
};

LoRa runtime;

#endif