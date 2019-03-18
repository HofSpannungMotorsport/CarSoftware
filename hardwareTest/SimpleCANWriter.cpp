#include "mbed.h"

#ifdef STM32F446xx
    #define CAN1_CONF PB_8,PB_9 // Car intern
#else
    //#define CAN1_CONF PD_0,PD_1 // Car intern
    #define CAN1_CONF PB_12,PB_6 // Motor Controller
#endif

CAN can1(CAN1_CONF, 500000);

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

void checkErrors() {
    if(can1.rderror() || can1.tderror()) {
        printf("\nCAN Errors:\n\tRD: %i\n\tTD: %i\n\n", can1.rderror(), can1.tderror());
        can1.reset();
    }
}

void SimpleCANWriter() {
    printf("SimpleCANWriter\n\n");
    can1.attach(receivedMessage);
    Ticker errorChecker;
    errorChecker.attach(&checkErrors, 2);

    wait(0.01);
    CANMessage msg;
    msg.format = CANStandard;

    msg.id = 0x201;
    msg.len = 3;
    msg.data[0] = 0x3D;
    msg.data[1] = 0x30;
    msg.data[2] = 0x00;

    can1.write(msg);

    while(1) {}
}