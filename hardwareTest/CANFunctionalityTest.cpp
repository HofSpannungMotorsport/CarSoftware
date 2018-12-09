// Copyed from https://os.mbed.com/handbook/CAN and modifyed

#include "mbed.h"
#include "../src/can/can_config.h"

#ifndef STM32F446xx
    #error "CANFunctionalitaTest written for NUCLEO-F446RE"
#endif

Ticker ticker;
DigitalOut led1(LED2);
CAN can1(CAN1_CONF, 250000);
char counter = 0;

void send() {
    printf("send()\n");
    if(can1.write(CANMessage(1337, &counter, 1))) {
        printf("wloop()\n");
        counter++;
        printf("Message sent: %d\n\n", counter);
        led1 = !led1;
    } else {
        printf("wloop() error\n\n");
    }
}

void CANFunctionalityTest() {
    printf("main()\n\n");
    ticker.attach(&send, 2);
    CANMessage msg;
    while(1) {
        printf("loop()\n");
        if(can1.read(msg)) {
            printf("Message received: %d\n", msg.data[0]);
        }

        if(can1.rderror() || can1.tderror()) {
            printf("CAN Errors:\n\tRD: %i\n\tTD: %i\n\n", can1.rderror(), can1.tderror());
            can1.reset();
        } else {
            printf("\n");
        }
        wait(0.5);
    }
}