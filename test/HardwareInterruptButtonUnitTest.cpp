#include "../src/car/Car.h"

#ifdef BOARD_DASHBOARD

HardwareInterruptButton testButton(USER_BUTTON);

int HardwareInterruptButtonUnitTest()
{
    // Button Unit Test
    // Printout the different Stats for the Button
    printf("Button Unit Test\n");
    while(1) {
        if (testButton.getClick()) {
            printf("Clicked!\n");
        }

        if (testButton.getLongClick()) {
            printf("LongClicked!\n");
        }

        if (testButton.getPress()) {
            printf("Pressed!\n");
        }

        if (testButton.getLongClickStart()) {
            printf("LongClick started!\n");
        }
    }
}

#endif