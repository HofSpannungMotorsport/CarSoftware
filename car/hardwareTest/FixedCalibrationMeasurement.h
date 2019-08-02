#include "Bonnie2019/hardware/Pins_Pedal_NEW_PCB.h"
#include "carpi.h"

HardwarePedal gas(PEDAL_PIN_ROTATION_ANGLE_GAS_1, PEDAL_PIN_ROTATION_ANGLE_GAS_2, COMPONENT_PEDAL_GAS);
HardwarePedal brake(PEDAL_PIN_ROTATION_ANGLE_BRAKE, COMPONENT_PEDAL_BRAKE);

#define CALIBRATION_TIME 15

void FixedCalibrationMeasurement() {
    wait(0.3);
    pcSerial.printf("FixedCalibrationMeasurement\n\nStarting with Calibration in 5 \t");
    wait(1);
    
    for(int i = 4; i >= 0; i--) {
        pcSerial.printf("%i\t", i);

        if (i != 0) {
            wait(1);
        } else {
            pcSerial.printf("\n");
        }
    }

    pcSerial.printf("Starting Calibration for %i seconds!\n", CALIBRATION_TIME);

    wait(0.1);

    gas.setCalibrationStatus(CURRENTLY_CALIBRATING);
    brake.setCalibrationStatus(CURRENTLY_CALIBRATING);

    for (int i = CALIBRATION_TIME; i > 0; i--) {
        pcSerial.printf("%i ", i);
        wait(1);
    }

    gas.setCalibrationStatus(CURRENTLY_NOT_CALIBRATING);
    brake.setCalibrationStatus(CURRENTLY_NOT_CALIBRATING);

    pcSerial.printf("\nFinish!\n\nCalibration Data:\n");

    

    pedal_calibration_data_t gasCalibration;
    pedal_calibration_data_t brakeCalibration;
    
    bool calibrationOk = gas.getCalibration(gasCalibration) && brake.getCalibration(brakeCalibration);

    if (calibrationOk) {
        pcSerial.printf("#define STD_GAS_1_MIN %i\n#define STD_GAS_1_MAX %i\n\n", gasCalibration.min[0], gasCalibration.max[0]);
        pcSerial.printf("#define STD_GAS_2_MIN %i\n#define STD_GAS_2_MAX %i\n\n", gasCalibration.min[1], gasCalibration.max[1]);
        pcSerial.printf("#define STD_BRAKE_MIN %i\n#define STD_BRAKE_MAX %i\n\n", brakeCalibration.min[0], brakeCalibration.max[0]);
    } else {
        pcSerial.printf("Got calibration Error. Please try again!\nGas Error Code: 0x%x\nBrake Error Code: 0x%x\n", gas.getStatus(), brake.getStatus());
        while(true);
    }

    wait(5);

    pcSerial.printf("\nTo start again, press both pedals more than 50%%\n");

    while (gas.getValue() < 0.5 || brake.getValue() < 0.5);

    pcSerial.printf("OK! Restart...\n\n");
}