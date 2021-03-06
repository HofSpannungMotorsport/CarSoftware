// Copyed from https://os.mbed.com/handbook/CAN and modifyed

#include "mbed.h"

#ifdef STM32F446xx
    #define CAN1_CONF PB_8,PB_9 // Car intern
#else
    //#define CAN1_CONF PD_0,PD_1 // Car intern
    #define CAN1_CONF PB_12,PB_6 // Motor Controller
#endif

CAN can1(CAN1_CONF, 250000);

void receivedMessage() {
    CANMessage msg;
    while (can1.read(msg)) {
        pcSerial.printf("Message Received:\n\tm.id: 0x%x\n\tm.len: %i", msg.id, msg.len);
        for (uint8_t i = 0; i < msg.len; i++) {
            pcSerial.printf("\n\tm.data[%i]: 0x%x", i, msg.data[i]);
        }
        pcSerial.printf("\n\n");
    }
}

void CANSniffer() {
    printf("CANSniffer listening\n\n");
    can1.attach(receivedMessage);
    while(1) {
        if(can1.rderror() || can1.tderror()) {
            printf("\nCAN Errors:\n\tRD: %i\n\tTD: %i\n\n", can1.rderror(), can1.tderror());
            can1.reset();
        }
        wait(1);
    }
}