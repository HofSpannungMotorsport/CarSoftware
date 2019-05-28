#ifndef HARDWARE_SD_CARD_STRESS_TEST_H
#define HARDWARE_SD_CARD_STRESS_TEST_H

#include "carpi.h"
#include "../../Bonnie2019/hardware/Pins_Master.h"

#include "helper/DummySCar.h"
#include "helper/SynteticSync.h"

#define LOOP_WAIT_TIME 0.0000001
#define TEST_LOGGER_REFRESH_RATE 9999999
#define TEST_LOGGER_TIME 100 // s

//HardwareSDCard hardwareSD(COMPONENT_SYSTEM_SD_CARD, MASTER_PIN_SPI_SD_MOSI, MASTER_PIN_SPI_SD_MISO, MASTER_PIN_SPI_SD_SCK, MASTER_PIN_SPI_SD_CS);
HardwareSDCard hardwareSD(COMPONENT_SYSTEM_SD_CARD, D11, D12, D13, A3);

SynteticSync syncer(DEVICE_MASTER);
DummySCar dummySCar(syncer);
PLogger logger(dummySCar, hardwareSD);

HardwareAlive alive(COMPONENT_ALIVE_MASTER, LED2);

bool HardwareSDCardStressTest() {
    wait(5);
    alive.setAlive(false);

    static bool _begin = true;
    if (_begin) {
        _begin = false;

        pcSerial.printf("HardwareSDCardTest\n\nBegin SD: ");
        if (hardwareSD.begin()) {
            pcSerial.printf("Success!\n");
        } else {
            pcSerial.printf("SD Card can't be initialized!\n");
            return false;
        }
    } else {
        pcSerial.printf("SD Card Status: ");
            if (hardwareSD.getStatus() == 0) {
            pcSerial.printf("Success!\n");
        } else {
            pcSerial.printf("Error! sdCard Status Code: 0x%x\n", hardwareSD.getStatus());
            return false;
        }
    }

    pcSerial.printf("Open File: ");
    if (hardwareSD.open()) {
        pcSerial.printf("Success!\n");
    } else {
        pcSerial.printf("File can't be opened!\n");
    }

    pcSerial.printf("Check SD Status: ");
    if (hardwareSD.getStatus() == 0) {
        pcSerial.printf("OK\n");
    } else {
        pcSerial.printf("Error! sdCard Status Code: 0x%x\n", hardwareSD.getStatus());
        return false;
    }

    static bool _added = false;
    if (!_added) {
        pcSerial.printf("Adding alive as LoggableValue with %i refreshrate to logger and begin: ", TEST_LOGGER_REFRESH_RATE);
        logger.addLogableValue(alive, SD_LOG_ID_ALIVE, TEST_LOGGER_REFRESH_RATE);
        logger.finalize();
        _added = true;

        if (logger.begin()) {
            pcSerial.printf("Success!\n");
        } else {
            pcSerial.printf("Error!\n");
            return false;
        }
    }

    dummySCar.setState(READY_TO_DRIVE); // That the logger really logs


    pcSerial.printf("Now let the logger run for %i seconds\n", TEST_LOGGER_TIME);
    Timer timer;
    timer.reset();
    timer.start();
    while (timer < TEST_LOGGER_TIME) {
        if (timer > (TEST_LOGGER_TIME/2) && alive.getAlive() == false) alive.setAlive(true);
        logger.run();
        wait(LOOP_WAIT_TIME);
    }

    hardwareSD.close();
    pcSerial.printf("\nFinish! Check SD Card for content\n\n");
    wait(30);

    return true;
}

#endif // HARDWARE_SD_CARD_STRESS_TEST_H