#include "mbed.h"

//#define CAR_BONNIE_2019   // [CAR_BONNIE_2019] -> Which car will this be compiled for?
//#define BOARD_Master      // [BOARD_DASHBOARD:BOARD_PEDAL:BOARD_MASTER] -> Which type of board will this be compiled for?
#include "car/Car.h"

#include "../test/HardwareInterruptButtonUnitTest.cpp"

int main()
{
    HardwareInterruptButtonUnitTest();

    while(1) {
        
    }
}