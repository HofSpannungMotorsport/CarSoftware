#include "../src/components/hardware/HardwareInterruptButton.h"
#include "../src/components/software/SoftwareButton.h"
#include "../src/can/ButtonMessageHandler.h"

HardwareInterruptButton testButton = HardwareInterruptButton(USER_BUTTON, NORMALLY_CLOSED);
SoftwareButton testSoftwareButton = SoftwareButton();

ButtonMessageHandler buttonHandler;

void ButtonUnitTest() {
    // Button Unit Test
    // Printout the different Stats for the Button
    // Testing HardwareInterruptButton and SoftwareButton
    pcSerial.printf("Button Unit Test\n");
    while(1) {
        Timer test = Timer();
        test.start();
        if (testButton.getStateChanged()) {
            CANMessage message = CANMessage();

            void *hardwareInterruptButtonToHandle = &testButton;
            void *softwareButtonToHandle = &testSoftwareButton;

            buttonHandler.buildMessage(hardwareInterruptButtonToHandle, message);
            buttonHandler.parseMessage(softwareButtonToHandle, message);
        }
        test.stop();

        if (testSoftwareButton.getStateChanged()) {
            button_state_t currentState = testSoftwareButton.getState();

            pcSerial.printf("Button ");

            if (currentState == NOT_PRESSED) {
                pcSerial.printf("Released!\n");
            }

            if (currentState == LONG_CLICKED) {
                pcSerial.printf("LongClickStarted!\n");
            }

            if (currentState == PRESSED) {
                pcSerial.printf("Pressed!\n");
            }

            pcSerial.printf("Time to generate Message: %f\n", test.read());
        }
    }
}