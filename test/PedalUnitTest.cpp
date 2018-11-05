#include "../src/components/hardware/HardwareInterruptButton.h"
#include "../src/components/hardware/HardwarePedal.h"

#define PEDAL_PIN1 A3
#define PEDAL_PIN2 A4

#define REFRESH_TIME 28 // ms

HardwareInterruptButton calibrationButton = HardwareInterruptButton(USER_BUTTON, NORMALLY_CLOSED);
HardwarePedal pedal = HardwarePedal(PEDAL_PIN1, PEDAL_PIN2);

void PedalUnitTest() {
    // Pedal Unit Test
    // Calibrate and test a Pedal
    printf("Pedal Unit Test\n");

    while (calibrationButton.getState() != PRESSED);

    printf("Calibration begin");
    pedal.beginCalibration();

    while (calibrationButton.getState() != NOT_PRESSED);
    while (calibrationButton.getState() != PRESSED);

    printf("Calibration end");
    pedal.endCalibration();

    if (pedal.getStatus() > 0) {
        printf("Error after Pedal Calibration: %u", pedal.getStatus());
    } else {
        Timer refreshTimer = Timer();
        refreshTimer.start();
        while(pedal.getStatus() == 0) {
            refreshTimer.reset();
            printf("Pedal Value: %.3f", pedal.getValue());
            while(refreshTimer.read_ms() < REFRESH_TIME);
        }

        printf("Pedal Error: %u", pedal.getStatus());
    }

    printf("End of Program");
    while(1);
}