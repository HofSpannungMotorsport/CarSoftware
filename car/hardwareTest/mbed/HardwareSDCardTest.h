#ifndef HARDWARE_SD_CARD_TEST_H
#define HARDWARE_SD_CARD_TEST_H

#include "carpi.h"
#include "../../Bonnie2019/hardware/Pins_Master.h"

#include "helper/DummySCar.h"
#include "helper/SynteticSync.h"

#define LOOP_WAIT_TIME 0.0001

HardwareSDCard hardwareSD(COMPONENT_SYSTEM_SD_CARD, MASTER_PIN_SPI_SD_MOSI, MASTER_PIN_SPI_SD_MISO, MASTER_PIN_SPI_SD_SCK, MASTER_PIN_SPI_SD_CS);

SynteticSync syncer(DEVICE_MASTER);
DummySCar dummySCar(syncer);
PLogger logger(dummySCar, hardwareSD);

HardwareAlive alive(COMPONENT_ALIVE_MASTER, LED2);

void HardwareSDCardTest() {
    pcSerial.printf("HardwareSDCardTest\n\nBegin SD: ");
    if (hardwareSD.begin()) {
        pcSerial.printf("Success!\n");
    } else {
        pcSerial.printf("Error!\n");
        while(true);
    }

    pcSerial.printf("Writing custom String: ");

    ISDCard* sdCard = &hardwareSD;
    string testString = "Custom Test String";
    sdCard->write(hardwareSD, 99, testString);

    if (hardwareSD.getStatus() == 0) {
        pcSerial.printf("Success!\n");
    } else {
        pcSerial.printf("Error! sdCard Status Code: 0x%x\n", hardwareSD.getStatus());
        while(true);
    }

    pcSerial.printf("Adding alive as LoggableValue to logger and begin: \n");
    logger.addLogableValue(alive, SD_LOG_ID_ALIVE);
    logger.finalize();

    if (logger.begin()) {
        pcSerial.printf("Success!\n");
    } else {
        pcSerial.printf("Error!\n");
        while(true);
    }

    dummySCar.setState(READY_TO_DRIVE); // That the logger really logs


    pcSerial.printf("Now let logger run for 10 seconds\n");
    Timer timer;
    timer.start();
    while (timer < 10) {
        logger.run();
        wait(LOOP_WAIT_TIME);
    }

    pcSerial.printf("Finish! Check SD Card for content\n\n");

    while(true);
}

#endif // HARDWARE_SD_CARD_TEST_H