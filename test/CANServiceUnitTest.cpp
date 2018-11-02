#include "../src/components/hardware/HardwareInterruptButton.h"
#include "../src/components/software/SoftwareButton.h"
#include "../src/can/ButtonMessageHandler.h"

#define CONTROLLER 1 // 1 = Transmitter/2 = Receiver


ButtonMessageHandler buttonHandler;

#if CONTROLLER == 1
HardwareInterruptButton testButton = HardwareInterruptButton(USER_BUTTON, BUTTON_START, NORMALLY_CLOSED);
#endif
#if CONTROLLER == 2
SoftwareButton testButton = SoftwareButton(BUTTON_START);
#endif

void CANServiceUnitTest() {

    // CANService Unit Test
    // Printout the different Stats for the Button
    printf("Button Unit Test\n");

    void* testButtonPointer = &testButton;
    IMessageHandler<CANMessage>* buttonHandlerPointer = &buttonHandler;
    canService.addComponent(testButtonPointer, buttonHandlerPointer);


    while(1) {

#if CONTROLLER == 1
        if (testButton.getStateChanged()) {
            canService.sendMessage(testButtonPointer);
        }
#endif
#if CONTROLLER == 2

    canService.processInbound();

    if (testButton.getStateChanged()) {
            button_state_t currentState = testButton.getState();

            printf("Button ");

            if (currentState == NOT_PRESSED) {
                printf("Released!\n");
            }

            if (currentState == LONG_CLICKED) {
                printf("LongClickStarted!\n");
            }

            if (currentState == PRESSED) {
                printf("Pressed!\n");
            }
        }
#endif
    }
}