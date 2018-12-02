#include "../src/components/hardware/HardwareInterruptButton.h"
#include "../src/components/hardware/HardwareLed.h"

#define LED_PIN LED2

#define REFRESH_TIME 28 // ms

HardwareInterruptButton calibrationButton = HardwareInterruptButton(USER_BUTTON, NORMALLY_CLOSED);
HardwareLed led = HardwareLed(LED_PIN);

#ifndef MESSAGE_REPORT
    Serial pcSerial(USBTX, USBRX); // Connection to PC over Serial
#endif

void LEDUnitTest() {
    // LED Unit Test
    // Go throu the different LED Modes by pressing the button
    pcSerial.printf("LED Unit Test\n");

    led.setState(LED_ON);

    while(1) {
        while (calibrationButton.getState() != NOT_PRESSED);
        while (calibrationButton.getState() != PRESSED);

        led.setBlinking(BLINKING_SLOW);

        while (calibrationButton.getState() != NOT_PRESSED);
        while (calibrationButton.getState() != PRESSED);

        led.setBlinking(BLINKING_NORMAL);

        while (calibrationButton.getState() != NOT_PRESSED);
        while (calibrationButton.getState() != PRESSED);

        led.setBlinking(BLINKING_FAST);

        while (calibrationButton.getState() != NOT_PRESSED);
        while (calibrationButton.getState() != PRESSED);

        led.setBlinking(BLINKING_OFF);
    }
}