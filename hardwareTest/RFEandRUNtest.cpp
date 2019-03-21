#include "../src/can/can_ids.h"
#include "../src/car/Bonnie2019/hardware/Pins_Master.h"
#include "../src/components/hardware/HardwareInterruptButton.h"
#include "mbed.h"
#include <string>


HardwareInterruptButton buttonNext(MASTER_PIN_USER_BUTTON, BUTTON_START);

// Dout Enable Pins for Motor Controller
DigitalOut enableRFE(MASTER_PIN_RFE_ENABLE);
DigitalOut enableRUN(MASTER_PIN_RUN_ENABLE);

void waitForClick() {
    while(buttonNext.getState() != NOT_PRESSED);
    while(buttonNext.getState() != PRESSED);
}

void testPin(string deviceName, DigitalOut &device) {
    string message = "Click button to turn On:  " + deviceName + "\n";
    pcSerial.printf("%s", message.c_str());
    waitForClick();
    device = 1;

    message = "Click button to turn Off: " + deviceName + "\n";
    pcSerial.printf("%s", message.c_str());
    waitForClick();
    device = 0;
}

void RFEandRUNtest() {
    // At first, release all
    enableRFE = 0;
    enableRUN = 0;

    pcSerial.printf("Click button to turn On:  RFE\n");
    waitForClick();
    enableRFE = 1;

    pcSerial.printf("Click button to turn On:  RUN\n");
    waitForClick();
    enableRUN = 1;

    pcSerial.printf("Click button to turn Off: RFE and RUN\n\n");
    waitForClick();
    enableRFE = 0;
    enableRUN = 0;
}