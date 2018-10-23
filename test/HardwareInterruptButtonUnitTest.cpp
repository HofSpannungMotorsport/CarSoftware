#define CAR_BONNIE_2019
#define BOARD_DASHBOARD
#include "car/Car.h"

int HardwareInterruptButtonUnitTest()
{
    // Button Unit Test
    // Printout the different Stats for the Button
    printf("Button Unit Test\n");
    while(1) {
        if (buttonLower.getClick()) {
            printf("Clicked!\n");
        }

        if (buttonLower.getLongClick()) {
            printf("LongClicked!\n");
        }

        if (buttonLower.getPress()) {
            printf("Pressed!\n");
        }

        if (buttonLower.getLongClickStart()) {
            printf("LongClick started!\n");
        }
    }
}