/*
    Simple Analog Value Reader. It reads the voltage on an analog pin,
    then prints it to serial and set a led to match the voltage on the pin.

    To better see the Results, use a serial Plotter (like the Arduino Serial Plotter).
*/

#include "mbed.h"

#define PIN PB_0
#define PIN2 PC_1
#define PIN3 PC_0
#define REFRESH_TIME 0.02 // s

AnalogIn analogPin(PIN);
AnalogIn analogPin2(PIN2);
AnalogIn analogPin3(PIN3);
PwmOut led(LED2);


Ticker analogRead;
uint16_t currentAnalogValue,
         currentAnalogValue2,
         currentAnalogValue3;

void analogReadToSerial() {
    pcSerial.printf("%i\t%i\t%i\n", currentAnalogValue, currentAnalogValue2, currentAnalogValue3);
}

void AnalogRead() {
    pcSerial.printf("AnalogRead Test\n\n");
    analogRead.attach(analogReadToSerial, REFRESH_TIME);

    while(true) {
        currentAnalogValue = analogPin.read_u16();
        currentAnalogValue2 = analogPin2.read_u16();
        currentAnalogValue3 = analogPin3.read_u16();
        led.write((float)currentAnalogValue/65535);
    }
}