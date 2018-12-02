#include "mbed.h"

#define pin A4
#define REFRESH_TIME 333 // ms
#define REFRESH_TIME 333 // ms

AnalogIn analogPin(pin);

#ifndef MESSAGE_REPORT
    Serial pcSerial(USBTX, USBRX); // Connection to PC over Serial
#endif

void FreeTest() {
    Timer refreshTimer;
    refreshTimer.reset();
    refreshTimer.start();

    pcSerial.printf("Analog Value: %i\n", analogPin.read_u16());

    while(refreshTimer.read_ms() < REFRESH_TIME);
}