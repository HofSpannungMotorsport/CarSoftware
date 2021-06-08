//#define PEDAL_MESSAGE_HANDLER_DEBUG
#include "carpi.h"

#define PEDAL_PIN1 PC_0
#define PEDAL_PIN2 PC_1

#define REFRESH_TIME 20 // ms

HardwareInterruptButton calibrationButton(USER_BUTTON);

HardwarePedal hardwarePedal(PEDAL_PIN1, /*PEDAL_PIN2,*/ COMPONENT_PEDAL_BRAKE);

void PedalUnitTest() {
    // Pedal Unit Test
    // Calibrate and test a Pedal
    pcSerial.printf("Pedal Unit Test\n");

    while (calibrationButton.getState() != PRESSED);

    pcSerial.printf("Calibration begin\n");
    hardwarePedal.setCalibrationStatus(CURRENTLY_CALIBRATING);

    while (calibrationButton.getState() != NOT_PRESSED);
    while (calibrationButton.getState() != PRESSED);

    pcSerial.printf("Calibration end\n");
    hardwarePedal.setCalibrationStatus(CURRENTLY_NOT_CALIBRATING);

    if (hardwarePedal.getStatus() > 0) {
        pcSerial.printf("Error after Pedal Calibration: 0x%x", hardwarePedal.getStatus());
    } else {
        Timer refreshTimer;
        refreshTimer.start();
        while(hardwarePedal.getStatus() == 0) {
            refreshTimer.reset();

            pcSerial.printf("%.4f\n", hardwarePedal.getValue());
            while(refreshTimer.read_ms() < REFRESH_TIME);
        }

        pcSerial.printf("Pedal Error: 0x%x\n", hardwarePedal.getStatus());
    }

    pcSerial.printf("\n\n-----\nEnd of Program");
    while(1);
}