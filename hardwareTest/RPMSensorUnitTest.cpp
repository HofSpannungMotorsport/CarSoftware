#include "../src/components/hardware/HardwareRpmSensor.h"
#include "../src/components/software/SoftwareRpmSensor.h"
#include "../src/can/RpmSensorMessageHandler.h"

#define RPM_PIN A0
#define REFRESH_RATE 3 // Hz

HardwareRpmSensor rpmSensor(RPM_PIN, RPM_FRONT_LEFT);
SoftwareRpmSensor softwareRpmSensor(RPM_FRONT_LEFT);
RpmSensorMessageHandler rpmSensorMessageHandler;

void RPMSensorUnitTest() {
    pcSerial.printf("RPM Sensor Unit Test\n\n");

    while(1) {
        CANMessage message = CANMessage();
        rpmSensorMessageHandler.buildMessage((void*)&rpmSensor, message);
        rpmSensorMessageHandler.parseMessage((void*)&softwareRpmSensor, message);

        pcSerial.printf("Speed: %.6f\n", softwareRpmSensor.getFrequency());
        wait(1.0/(float)REFRESH_RATE);
    }
}