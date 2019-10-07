#include "Bonnie2019/hardware/Pins_Pedal_NEW_PCB.h"

#define REFRESH_INTERVAL 0.15

AnalogIn gas1(PEDAL_PIN_ROTATION_ANGLE_GAS_1);
AnalogIn gas2(PEDAL_PIN_ROTATION_ANGLE_GAS_2);
AnalogIn brake(PEDAL_PIN_ROTATION_ANGLE_BRAKE);

void PedalMagnetCalibration() {
    wait(3);
    pcSerial.printf("PedalMagnetCalibration\n\n");
    pcSerial.printf("Values given in a Range from 0.0 to 1.0 but scaled from 0.0 V to 5.0 V\n");
    pcSerial.printf("The Voltage represents the Voltage of the Sensor, which goes throu a voltage devider to be between 0.0 V and 3.3 V for the Microcontroller.\n");
    pcSerial.printf("The Sensor voltage should be in 0%%-Position between 1.1 V and 1.3 V, in 100%% Position not over 4.0 V, but please use your Brain. Thanks. Du Sack.\n\n");
    
    wait(5);

    while(true) {
        float x1 = gas1 * 5.0;
        float x2 = gas2 * 5.0;
        float x3 = brake * 5.0;
        pcSerial.printf("Gas 1: %.3f V\tGas 2: %.3f V\tBrake: %.3f V\n", x1, x2, x3);
        wait(REFRESH_INTERVAL);
    }
}