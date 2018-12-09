#include "../src/components/hardware/HardwareInterruptButton.h"
#include "../src/components/hardware/HardwarePedal.h"

#include "../src/can/can_ids.h"
#include "../src/components/interface/IID.h"

#define PEDAL_PIN1 A3
#define PEDAL_PIN2 A5

#define REFRESH_TIME 8 // ms

HardwareInterruptButton calibrationButton(USER_BUTTON);

HardwarePedal pedal(PEDAL_PIN1, PEDAL_PIN2, PEDAL_GAS);

#ifndef MESSAGE_REPORT
    #define MESSAGE_REPORT
    Serial pcSerial(USBTX, USBRX); // Connection to PC over Serial
#endif

void HardwarePedalUnitTest() {
    // Pedal Unit Test
    // Calibrate and test a Pedal
    pcSerial.printf("Pedal Unit Test\n");

    while (calibrationButton.getState() != PRESSED);

    pcSerial.printf("Calibration begin\n");
    pedal.setCalibrationStatus(CURRENTLY_CALIBRATING);

    while (calibrationButton.getState() != NOT_PRESSED);
    while (calibrationButton.getState() != PRESSED);

    pcSerial.printf("Calibration end\n");
    pedal.setCalibrationStatus(CURRENTLY_NOT_CALIBRATING);

    if (pedal.getStatus() > 0) {
        pcSerial.printf("Error after Pedal Calibration: 0x%x\n", pedal.getStatus());
    } else {
        Timer refreshTimer = Timer();
        refreshTimer.start();
        while(pedal.getStatus() == 0) {
            refreshTimer.reset();
            pcSerial.printf("%.3f\n", pedal.getValue());
            while(refreshTimer.read_ms() < REFRESH_TIME);
        }

        pcSerial.printf("Pedal Error: 0x%x\n", pedal.getStatus());
    }

    pcSerial.printf("\n\n-----\nEnd of Program");
    while(1);
}