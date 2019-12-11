#include "carpi.h"

#define PEDAL_PIN1 A3
#define PEDAL_PIN2 A5

#define REFRESH_TIME 8 // ms

HardwareInterruptButton calibrationButton(USER_BUTTON);

HardwarePedal pedal(PEDAL_PIN1, PEDAL_PIN2, PEDAL_GAS);

void HardwarePedalUnitTest() {
    // Pedal Unit Test
    // Calibrate and test a Pedal
    printf("Pedal Unit Test\n");

    while (calibrationButton.getState() != PRESSED);

    printf("Calibration begin\n");
    pedal.setCalibrationStatus(CURRENTLY_CALIBRATING);

    while (calibrationButton.getState() != NOT_PRESSED);
    while (calibrationButton.getState() != PRESSED);

    printf("Calibration end\n");
    pedal.setCalibrationStatus(CURRENTLY_NOT_CALIBRATING);

    if (pedal.getStatus() > 0) {
        printf("Error after Pedal Calibration: 0x%x\n", pedal.getStatus());
    } else {
        Timer refreshTimer;
        refreshTimer.start();
        while(pedal.getStatus() == 0) {
            refreshTimer.reset();
            printf("%.3f\n", pedal.getValue());
            while(refreshTimer.read_ms() < REFRESH_TIME);
        }

        printf("Pedal Error: 0x%x\n", pedal.getStatus());
    }

    printf("\n\n-----\nEnd of Program");
    while(1);
}