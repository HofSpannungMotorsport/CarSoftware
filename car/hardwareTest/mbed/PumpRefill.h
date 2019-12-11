#include "carpi.h"
#include "../../Bonnie2019/hardware/Pins_Master.h"

#define PUMP_TIME_UNTIL_MAX_POWER 5.0 // s


HardwareInterruptButton buttonNext(MASTER_PIN_USER_BUTTON, COMPONENT_BUTTON_START);

// Dout Pump enable
DigitalOut pumpEnable(MASTER_PIN_PUMP_ENABLE);

// PWMOut Pump
PwmOut pumpPWM(MASTER_PIN_PUMP_PWM);

// Cooling Fans
DigitalOut coolingFan(MASTER_PIN_FAN);


void waitForClick() {
    while(buttonNext.getState() != NOT_PRESSED);
    while(buttonNext.getState() != PRESSED);
}

void releaseAll() {
    pumpEnable = 0;
    pumpPWM = 1;
    coolingFan = 0;
}

void PumpRefill() {
    releaseAll();

    printf("Waiting for Button Click on Master (blue Button) to start Pump...\n\n");
    waitForClick();

    printf("Setting Pump Enable now\n");
    pumpEnable = 1;

    printf("Pump now primed. Starting pump in 2 Seconds... ");
    wait(2.0);

    printf("Starting now\n");

    {
        Timer timer;
        timer.reset();
        timer.start();

        float currentTime = 0.0;
        

        for (uint8_t i = 1; i <= 10; i++) {
            while(currentTime < PUMP_TIME_UNTIL_MAX_POWER/10*i) {
                pumpPWM = 1 - (currentTime / PUMP_TIME_UNTIL_MAX_POWER);
                currentTime = timer.read();
            }

            printf("%i0%%\n", i);
        }
    }

    pumpPWM = 0;

    printf("Pump now now fully throtteled\nClick Button to start Fan\n");
    waitForClick();

    coolingFan = 1;

    printf("Fan on\n\n");

    waitForClick();
}