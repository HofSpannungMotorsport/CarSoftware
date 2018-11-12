#include "../src/components/hardware/HardwareRpmSensor.h"

#define RPM_PIN A0

HardwareRpmSensor rpmSensor(RPM_PIN);

#ifndef MESSAGE_REPORT
    Serial pcSerial(USBTX, USBRX); // Connection to PC over Serial
#endif

void RPMSensorUnitTest() {
    pcSerial.printf("RPM Sensor Unit Test\n\n");

    while(1) {
        pcSerial.printf("Speed: %.1f\n", rpmSensor.getFrequency());
    }
}