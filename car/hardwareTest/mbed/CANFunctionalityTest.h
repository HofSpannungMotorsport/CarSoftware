// Copyed from https://os.mbed.com/handbook/CAN and modifyed

#include "mbed.h"
#include "../Bonnie2019/hardware/Pins_Master.h"
#include "../Bonnie2019/hardware/Pins_Pedal.h"

#ifdef STM32F446xx
    #define CAN1_CONF PB_8,PB_9
    const uint16_t id = 1337;
#else
    #define CAN1_CONF PD_0, PD_1
    const uint16_t id = 1338;
#endif

Ticker ticker;
DigitalOut led1(LED2);
CAN can1(CAN1_CONF, 250000);
char counter = 0;

void send() {
    printf("send()\n");
    if(can1.write(CANMessage(id, &counter, 1))) {
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