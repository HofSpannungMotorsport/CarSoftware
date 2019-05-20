/*
    Small CAN-Service test to test its functionality
    The type of the controller (Transmitter/Receiver) is set by the CONTROLLER define

    This Test will send the button States from Transmitter to Receiver while the 
    Receiver will answer with the Control of the LED.
*/
#define CONTROLLER 1 // 1 = Transmitter/2 = Receiver

#include "mbed.h"

//#define CAN_DEBUG

#include "carpi.h"


CANService canService(CAN1_CONF, 1000000);
LEDMessageHandler ledMessageHandler;
ButtonMessageHandler buttonHandler;

#if CONTROLLER == 1
    HardwareInterruptButton testButton = HardwareInterruptButton(USER_BUTTON, BUTTON_START, NORMALLY_CLOSED);
    HardwareLed led(LED2, LED_READY_TO_DRIVE);
#endif
#if CONTROLLER == 2
    SoftwareButton testButton = SoftwareButton(BUTTON_START);
    SoftwareLed led(LED_READY_TO_DRIVE);
#endif

void CANServiceUnitTest() {

    // CANService Unit Test
    // Printout the different States for the Button
    pcSerial.printf("CANService Button Unit Test with LED Output\n\n");

    canService.addComponent((void*)&testButton, (IMessageHandler<CANMessage>*)&buttonHandler);
    canService.addComponent((void*)&led, (IMessageHandler<CANMessage>*)&ledMessageHandler);

    #if CONTROLLER == 2
        canService.addComponentToSendLoop((void*)&led);
        led.setState(LED_OFF);
        canService.run();
    #endif

    while(1) {
        #if CONTROLLER == 1
            canService.run();
            if (testButton.getStateChanged()) {
                canService.sendMessage((void*)&testButton);
            }
        #endif
        #if CONTROLLER == 2
            canService.processInbound();

            while (testButton.getStateChanged()) {
                button_state_t currentState = testButton.getState();

                pcSerial.printf("Button ");

                if (currentState == NOT_PRESSED) {
                    pcSerial.printf("NOT_PRESSED!\n");
                    led.setState(LED_OFF);
                }

                if (currentState == PRESSED) {
                    pcSerial.printf("PRESSED!\n");
                    led.setState(LED_ON);
                    led.setBlinking(BLINKING_OFF);
                }

                if (currentState == LONG_CLICKED) {
                    pcSerial.printf("LONG_CLICKED!\n");
                    led.setState(LED_ON);
                    led.setBlinking(BLINKING_FAST);
                }

                if (led.getSentConfigurationChanged()) {
                    canService.processSendLoop();
                }
            }
        #endif
        // wait(0.001);
    }
}