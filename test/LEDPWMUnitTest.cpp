#include "../src/components/hardware/HardwareInterruptButton.h"
#include "../src/components/hardware/HardwareLedPwm.h"

#define LED_PIN LED2

#define REFRESH_TIME 28 // ms

HardwareInterruptButton calibrationButton = HardwareInterruptButton(USER_BUTTON, NORMALLY_CLOSED);
HardwareLedPwm led = HardwareLedPwm(LED_PIN);

#ifndef MESSAGE_REPORT
    Serial pcSerial(USBTX, USBRX); // Connection to PC over Serial
#endif

void LEDPWMUnitTest() {
    // LED Unit Test
    // Go throu the different LED Modes by pressing the button
    pcSerial.printf("LED Unit Test\n");

    led.setState(LED_ON);

    uint16_t timeForFade = 1000;
    for (uint8_t i = 0; i < 255; i++) {
        led.setBrightness((float)i/254.0);
        wait_ms(timeForFade/254);
    }

    for (uint8_t i = 255; i > 0; i--) {
        led.setBrightness((float)i/254.0);
        wait_ms(timeForFade/254);
    }

    led.setBrightness(1);

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

        while (calibrationButton.getState() != NOT_PRESSED);
        while (calibrationButton.getState() != PRESSED);

        uint16_t timeForFade = 1000;
        for (uint8_t i = 0; i < 255; i++) {
            led.setBrightness((float)i/254.0);
            wait_ms(timeForFade/255);
        }

        for (uint8_t i = 255; i > 0; i--) {
            led.setBrightness((float)i/254.0);
            wait_ms(timeForFade/255);
        }

        led.setBrightness(1);
    }
}