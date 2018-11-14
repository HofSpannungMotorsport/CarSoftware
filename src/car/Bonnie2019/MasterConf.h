#ifndef BONNIE_2019_MASTER_CONF_H
#define BONNIE_2019_MASTER_CONF_H

#include "mbed.h"
#include "hardware/Pins_Master.h"
#include "../../components/hardware/HardwareLed.h"
#include "../../components/software/SoftwareLed.h"

SoftwareLed greenLed;
SoftwareLed yellowLed;
SoftwareLed redLed;

HardwareLed brakeLight(MASTER_PIN_BRAKE_LIGHT); // change pin

class Master {
    public:
        // Called once at bootup
        void setup() {
            carService.startUp();
        }

        // Called repeately after bootup
        void loop() {

        }
};

Master runtime;

#endif