#ifndef ALIVE_COMPONENT_TEST
#define ALIVE_COMPONENT_TEST

#include "helper/SynteticSync.h"

SynteticSync syncer(DEVICE_MASTER);

HardwareAlive hardwareAlive(COMPONENT_SUB_ID_1, LED2);
SoftwareAlive softwareAlive(COMPONENT_SUB_ID_2);

bool AliveComponentTest() {
    wait(1);

    printf("AliveComponentTest\n\n");

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
        printf("Alive\n");
    } else {
        printf("Not Alive -> Error\n");
        return false;
    }

    if (softwareAlive.getStatus() == 0) {
        printf("Alive\n");
    } else {
        printf("Not Alive -> Error\n");
        return false;
    }

    hardwareAlive.detach();

    timer.reset();
    while(timer < 2) {
        syncer.run();
        wait(0.001);
    }

    if (softwareAlive.getAlive()){
        printf("Still Alive -> Error\n");
        return false;
    } else {
        printf("Not Alive anymore!\n");
    }

    if (softwareAlive.getStatus() == 0){
        printf("Still Alive -> Error\n");
        return false;
    } else {
        printf("Not Alive anymore!\n");
    }

    hardwareAlive.attach(syncer);

    timer.reset();
    while(timer < 2) {
        syncer.run();
        wait(0.001);
    }

    if (softwareAlive.getAlive()) {
        printf("Alive again\n");
    } else {
        printf("Not Alive now -> Error\n");
        return false;
    }

    if (softwareAlive.getStatus() == 0) {
        printf("Alive again\n");
    } else {
        printf("Not Alive now -> Error\n");
        return false;
    }

    hardwareAlive.setAlive(false);

    timer.reset();
    while(timer < 2) {
        syncer.run();
        wait(0.001);
    }

    if (softwareAlive.getAlive()){
        printf("Still Alive after set -> Error\n");
        return false;
    } else {
        printf("Not Alive after set!\n");
    }

    if (softwareAlive.getStatus() == 0){
        printf("Still Alive after set -> Error\n");
        return false;
    } else {
        printf("Not Alive after set!\n");
    }

    return true;
}

#endif // ALIVE_COMPONENT_TEST