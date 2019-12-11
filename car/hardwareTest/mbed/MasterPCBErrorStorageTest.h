#include "mbed.h"
#include "Bonnie2019/hardware/Pins_Master_PCB.h"

DigitalOut microcontrollerOk(MASTER_PIN_MICROCONTROLLER_OK);

DigitalIn bspdError(MASTER_PIN_ERROR_BSPD);
DigitalIn bmsError(MASTER_PIN_ERROR_BMS);
DigitalIn imdError(MASTER_PIN_ERROR_IMD);

DigitalOut bspdLed(LED3); // red
DigitalOut bmsLed(LED2); // blue
DigitalOut imdLed(LED1); // green

void MasterPCBErrorStorageTest() {
    printf("MasterPCBErrorStorageTest\n");
    microcontrollerOk = 1;
    while(true) {
        bspdLed = bspdError;
        bmsLed = bmsError;
        imdLed = imdError;
        wait(0.1);
    }
}