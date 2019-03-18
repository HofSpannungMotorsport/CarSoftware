#include "../src/can/can_ids.h"
#include "../src/car/Bonnie2019/hardware/Pins_Master.h"
#include "../src/components/hardware/HardwareInterruptButton.h"
#include "mbed.h"

#define PUMP_TIME_UNTIL_MAX_POWER 5.0 // s


HardwareInterruptButton buttonNext(MASTER_PIN_USER_BUTTON, BUTTON_START);

// Dout Pump enable
DigitalOut pumpEnable(MASTER_PIN_PUMP_ENABLE);

// PWMOut Pump
PwmOut pumpPWM(MASTER_PIN_PUMP_PWM);

// Cooling Fans
DigitalOut coolingFans(MASTER_PIN_FAN);


void waitForClick() {
    while(buttonNext.getState() != NOT_PRESSED);
    while(buttonNext.getState() != PRESSED);
}

void releaseAll() {
    pumpEnable = 0;
    pumpPWM = 0;
    coolingFans = 0;
}

void PumpRefill() {
    releaseAll();

    pcSerial.printf("Waiting for Button Click on Master (blue Button) to start Pump...\n\n");
    waitForClick();

    pcSerial.printf("Setting Pump Enable now\n");
    pumpEnable = 1;

    pcSerial.prinf("Pump now primed. Starting pump in 2 Seconds... ");
    wait(2.0);

    pcSerial.printf("Starting now\n\n");

    {
        Timer timer;
        timer.reset();
        timer.start();

        float currentTime = 0.0;
        while(currentTime < PUMP_TIME_UNTIL_MAX_POWER) {
            pumpPWM = (currentTime / PUMP_TIME_UNTIL_MAX_POWER);
            currentTime = timer.read();
        }
    }

    pumpPWM = 1;


    while(true); // Wait forever
}