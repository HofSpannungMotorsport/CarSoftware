#include "carpi.h"

#define LED_PIN LED2

#define REFRESH_TIME 28 // ms

HardwareInterruptButton button = HardwareInterruptButton(USER_BUTTON, NORMALLY_CLOSED);
HardwareLedPwm hardwareLed = HardwareLedPwm(LED_PIN, LED_DASHBOARD);
SoftwareLed led = SoftwareLed(LED_DASHBOARD);
LEDMessageHandler ledMessageHandler;


void syncLed() {
    CANMessage msg;
    ledMessageHandler.buildMessage((void*)&led, msg);
    ledMessageHandler.parseMessage((void*)&hardwareLed, msg);
}

void LEDPWMUnitTest() {
    // LED Unit Test
    // Go throu the different LED Modes by pressing the button
    pcSerial.printf("LED PWM Unit Test\n");

    led.setState(LED_ON);
    syncLed();

    uint16_t timeForFade = 1000;
    for (uint8_t i = 0; i < 255; i++) {
        led.setBrightness((float)i/254.0);
        syncLed();
        wait_ms(timeForFade/254);
    }

    for (uint8_t i = 255; i > 0; i--) {
        led.setBrightness((float)i/254.0);
        syncLed();
        wait_ms(timeForFade/254);
    }

    led.setBrightness(1);
    syncLed();

    while(1) {
        while (button.getState() != NOT_PRESSED);
        while (button.getState() != PRESSED);

        led.setBlinking(BLINKING_SLOW);
        syncLed();

        while (button.getState() != NOT_PRESSED);
        while (button.getState() != PRESSED);

        led.setBlinking(BLINKING_NORMAL);
        syncLed();

        while (button.getState() != NOT_PRESSED);
        while (button.getState() != PRESSED);

        led.setBlinking(BLINKING_FAST);
        syncLed();

        while (button.getState() != NOT_PRESSED);
        while (button.getState() != PRESSED);

        led.setBlinking(BLINKING_OFF);
        led.setState(LED_OFF);
        syncLed();

        while (button.getState() != NOT_PRESSED);
        while (button.getState() != PRESSED);

        led.setBrightness(0);
        led.setState(LED_ON);
        syncLed();

        uint16_t timeForFade = 1000;
        for (uint8_t i = 0; i < 255; i++) {
            led.setBrightness((float)i/254.0);
            syncLed();
            wait_ms(timeForFade/255);
        }

        for (uint8_t i = 255; i > 0; i--) {
            led.setBrightness((float)i/254.0);
            syncLed();
            wait_ms(timeForFade/255);
        }

        wait(0.3);

        led.setBrightness(1);
        syncLed();
    }
}