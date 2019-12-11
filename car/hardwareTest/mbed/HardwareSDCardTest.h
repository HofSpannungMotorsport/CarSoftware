#ifndef HARDWARE_SD_CARD_TEST_H
#define HARDWARE_SD_CARD_TEST_H

#include "carpi.h"
#include "../../Bonnie2019/hardware/Pins_Master.h"

#include "helper/DummySCar.h"
#include "helper/SynteticSync.h"

#define LOOP_WAIT_TIME 0.000001

//HardwareSDCard hardwareSD(COMPONENT_SYSTEM_SD_CARD, MASTER_PIN_SPI_SD_MOSI, MASTER_PIN_SPI_SD_MISO, MASTER_PIN_SPI_SD_SCK, MASTER_PIN_SPI_SD_CS);
HardwareSDCard hardwareSD(COMPONENT_SYSTEM_SD_CARD, D11, D12, D13, A3);

SynteticSync syncer(DEVICE_MASTER);
DummySCar dummySCar(syncer);
PLogger logger(dummySCar, hardwareSD);

HardwareAlive alive(COMPONENT_ALIVE_MASTER, LED2);

bool HardwareSDCardTest() {
    wait(2);
    alive.setAlive(false);

    static bool _begin = true;
    if (_begin) {
        _begin = false;

        printf("HardwareSDCardTest\n\nBegin SD: ");
        if (hardwareSD.begin()) {
            printf("Success!\n");
        } else {
            printf("SD Card can't be initialized!\n");
            return false;
        }
    } else {
        printf("SD Card Status: ");
            if (hardwareSD.getStatus() == 0) {
            printf("Success!\n");
        } else {
            printf("Error! sdCard Status Code: 0x%x\n", hardwareSD.getStatus());
            return false;
        }
    }

    printf("Open File: ");
    if (hardwareSD.open()) {
        printf("Success!\n");
    } else {
        printf("File can't be opened!\n");
    }

    printf("Writing custom String: ");

    ISDCard* sdCard = &hardwareSD;
    string testString = "Custom Test String";
    sdCard->write(hardwareSD, 99, testString);

    if (hardwareSD.getStatus() == 0) {
        printf("Success!\n");
    } else {
        printf("Error! sdCard Status Code: 0x%x\n", hardwareSD.getStatus());
        return false;
    }

    static bool _added = false;
    if (!_added) {
        printf("Adding alive as LoggableValue to logger and begin: ");
        logger.addLogableValue(alive, SD_LOG_ID_ALIVE);
        logger.finalize();
        _added = true;

        if (logger.begin()) {
            printf("Success!\n");
        } else {
            printf("Error!\n");
            return false;
        }
    }

    dummySCar.setState(READY_TO_DRIVE); // That the logger really logs


    printf("Now let logger run for 5 seconds\n");
    Timer timer;
    timer.reset();
    timer.start();
    Timer dotTimer;
    dotTimer.reset();
    dotTimer.start();
    while (timer < 5) {
        if (timer > 2) alive.setAlive(true);
        logger.run();
        
        if (dotTimer > 0.5) {
            dotTimer.reset();
            printf(".");
        }

        wait(LOOP_WAIT_TIME);
    }

    hardwareSD.close();
    printf("\nFinish! Check SD Card for content\n\n");

    return true;
}

#endif // HARDWARE_SD_CARD_TEST_H