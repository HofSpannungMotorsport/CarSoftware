/*
    Simple Analog Value Reader. It reads the voltage on an analog pin,
    then prints it to serial and set a led to match the voltage on the pin.

    To better see the Results, use a serial Plotter (like the Arduino Serial Plotter).
*/

#include "mbed.h"

#define PIN A5
#define PIN2 A3
#define REFRESH_TIME 0.02 // s

AnalogIn analogPin(PIN);
AnalogIn analogPin2(PIN2);
PwmOut led(LED2);

#ifndef MESSAGE_REPORT
    Serial pcSerial(USBTX, USBRX); // Connection to PC over Serial
#endif

Ticker analogRead;
uint16_t currentAnalogValue;
uint16_t currentAnalogValue2;

void analogReadToSerial() {
    pcSerial.printf("%i\t%i\n", currentAnalogValue, currentAnalogValue2);
}

void AnalogRead() {
    pcSerial.printf("AnalogRead Test\n\n");
    analogRead.attach(analogReadToSerial, REFRESH_TIME);

    while(true) {
        currentAnalogValue = analogPin.read_u16();
        currentAnalogValue2 = analogPin2.read_u16();
        led.write((float)currentAnalogValue/65535);
    }
}