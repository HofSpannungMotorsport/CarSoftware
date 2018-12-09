#include "../src/components/hardware/HardwareInterruptButton.h"
#include "../src/components/hardware/HardwareLed.h"
#include "../src/components/software/SoftwareLed.h"
#include "../src/can/LEDMessageHandler.h"
#include "../src/can/can_ids.h"

#define LED_PIN LED2

#define REFRESH_TIME 28 // ms

HardwareInterruptButton button = HardwareInterruptButton(USER_BUTTON, NORMALLY_CLOSED);
HardwareLed hardwareLed = HardwareLed(LED_PIN, LED_DASHBOARD);
SoftwareLed led = SoftwareLed(LED_DASHBOARD);
LEDMessageHandler ledMessageHandler;


#ifndef MESSAGE_REPORT
    Serial pcSerial(USBTX, USBRX); // Connection to PC over Serial
#endif

void setLed(led_blinking_t mode) {
    CANMessage msg;
    led.setBlinking(mode);
    ledMessageHandler.buildMessage((void*)&led, msg);
    ledMessageHandler.parseMessage((void*)&hardwareLed, msg);
}

void LEDUnitTest() {
    // LED Unit Test
    // Go throu the different LED Modes by pressing the button
    pcSerial.printf("LED Unit Test\n");

    CANMessage msg;
    led.setState(LED_ON);
    ledMessageHandler.buildMessage((void*)&led, msg);
    ledMessageHandler.parseMessage((void*)&hardwareLed, msg);

    while(1) {
        while (button.getState() != NOT_PRESSED);
        while (button.getState() != PRESSED);

        setLed(BLINKING_SLOW);
        //hardwareLed.setBlinking(BLINKING_SLOW);

        while (button.getState() != NOT_PRESSED);
        while (button.getState() != PRESSED);

        setLed(BLINKING_NORMAL);
        //hardwareLed.setBlinking(BLINKING_NORMAL);

        while (button.getState() != NOT_PRESSED);
        while (button.getState() != PRESSED);

        setLed(BLINKING_FAST);
        //hardwareLed.setBlinking(BLINKING_FAST);

        while (button.getState() != NOT_PRESSED);
        while (button.getState() != PRESSED);

        setLed(BLINKING_OFF);
        //hardwareLed.setBlinking(BLINKING_OFF);
    }
}