#include "Bonnie2019/hardware/Pins_Pedal_NEW_PCB.h"

AnalogIn analogPedal(PEDAL_PIN_ROTATION_ANGLE_BRAKE);

void PedalAnalogRead() {
    wait(0.3);


    while(true) {
        printf("%.2f\n", analogPedal.read() * 100);
        wait(0.5);
    }
}