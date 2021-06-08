// Copyed from https://os.mbed.com/handbook/CAN and modifyed

#include "mbed.h"

CAN can1(D10,D2, 250000);

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

    Timer timer;
    timer.start();
    while(1) {
        if (timer > 1.0) {
            timer.reset();
            printf("Nix\n");
        }

        if(can1.rderror() || can1.tderror()) {
            printf("\nCAN Errors:\n\tRD: %i\n\tTD: %i\n\n", can1.rderror(), can1.tderror());
            can1.reset();
        }
        wait(1);
    }
}