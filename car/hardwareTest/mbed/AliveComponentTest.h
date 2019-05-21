#ifndef ALIVE_COMPONENT_TEST
#define ALIVE_COMPONENT_TEST

#include "helper/SynteticSync.h"

SynteticSync syncer(DEVICE_MASTER);

HardwareAlive hardwareAlive(COMPONENT_SUB_ID_1, LED2);
SoftwareAlive softwareAlive(COMPONENT_SUB_ID_2);

void AliveComponentTest() {
    wait(1);

    pcSerial.printf("AliveComponentTest\n\n");

    syncer.addComponent1(hardwareAlive);
    syncer.addComponent2(softwareAlive);

    hardwareAlive.attach(syncer);
    softwareAlive.attach(syncer);

    hardwareAlive.setAlive(true);

    Timer timer;
    timer.start();
    while(timer < 2) {
        syncer.run();
        wait(0.001);
    }

    if (softwareAlive.getAlive()) {
        pcSerial.printf("Alive\n");
    } else {
        pcSerial.printf("Not Alive -> Error\n");
    }

    if (softwareAlive.getStatus() == 0) {
        pcSerial.printf("Alive\n");
    } else {
        pcSerial.printf("Not Alive -> Error\n");
    }

    hardwareAlive.detach();

    timer.reset();
    while(timer < 2) {
        syncer.run();
        wait(0.001);
    }

    if (softwareAlive.getAlive()){
        pcSerial.printf("Still Alive -> Error\n");
    } else {
        pcSerial.printf("Not Alive anymore!\n");
    }

    if (softwareAlive.getStatus() == 0){
        pcSerial.printf("Still Alive -> Error\n");
    } else {
        pcSerial.printf("Not Alive anymore!\n");
    }

    hardwareAlive.attach(syncer);

    timer.reset();
    while(timer < 2) {
        syncer.run();
        wait(0.001);
    }

    if (softwareAlive.getAlive()) {
        pcSerial.printf("Alive again\n");
    } else {
        pcSerial.printf("Not Alive now -> Error\n");
    }

    if (softwareAlive.getStatus() == 0) {
        pcSerial.printf("Alive again\n");
    } else {
        pcSerial.printf("Not Alive now -> Error\n");
    }

    hardwareAlive.setAlive(false);

    timer.reset();
    while(timer < 2) {
        syncer.run();
        wait(0.001);
    }

    if (softwareAlive.getAlive()){
        pcSerial.printf("Still Alive after set -> Error\n");
    } else {
        pcSerial.printf("Not Alive after set!\n");
    }

    if (softwareAlive.getStatus() == 0){
        pcSerial.printf("Still Alive after set -> Error\n");
    } else {
        pcSerial.printf("Not Alive after set!\n");
    }

    while(true) {}
}

#endif // ALIVE_COMPONENT_TEST