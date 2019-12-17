#include <unity.h>
#include <cstring>

#define STEROIDO_DISABLE_LOOP
#include "steroido/src/Steroido.h"


// Explicitly include CANMessage even it there is no CAN
#include "steroido/src/Communication/CANMessage.h"

#define SYNC_H // Uninclude Sync
#include "runable/IRunable.h"
#include "communication/CarMessage.h"
#include "communication/IChannel.h"
#include "components/interface/ICommunication.h"

#define TEST_MESSAGE_COUNT 128 // Less than message queue of CCan!
//#define ENABLE_ADVANCED_PRINT

// Synthetic classes for Testing
void printCarMessage(CarMessage &carMessage) {
    // Print Constant Stuff
    #ifdef ENABLE_ADVANCED_PRINT
    printf("Sender ID: 0x%x | Receiver ID: 0x%x | Component ID: 0x%x\n", carMessage.getSenderId(), carMessage.getReceiverId(), carMessage.getComponentId());

    // Print Data
    printf("\tCarMessage with length %i", carMessage.getLength());
    for(uint8_t i = 0; i < carMessage.getLength(); i++) {
        printf(" | data[%i]: 0x%x", i, carMessage[i]);
    }

    printf("\n");
    #endif
}

class Sync : public IRunable {
    public:
        Sync() {}

        vector<CarMessage> receivedCarMessages;

        void receive(CarMessage &carMessage) {
            #ifdef ENABLE_ADVANCED_PRINT
            printf("[Sync]: Received CarMessage:\n");
            #endif
            printCarMessage(carMessage);

            receivedCarMessages.push_back(carMessage);
        }

        void clear() {
            receivedCarMessages.clear();
        }

        virtual void run() {
            // Do Nothing
        }
};

typedef int PinName;

void printCANMessage(CANMessage &canMessage) {
    #ifdef ENABLE_ADVANCED_PRINT
    printf("ID: 0x%x | len: %i", canMessage.id, canMessage.len);

    for (uint8_t i = 0; i < canMessage.len; i++) {
        printf(" | data[%i]: 0x%x", i, canMessage.data[i]);
    }
    printf("\n");
    #endif
}

void *canInstance;

// Using the "Arduino-Type" of the class, without the callback and attach method
class CAN {
    public:
        CAN(PinName rx, PinName tx, int hz) {
            writtenMsgCount = 0;
            maxMsgInWriteBuffer = 999;

            canInstance = this;
        }

        bool available() {
            return !messageInBuffer.empty();
        }

        bool read(CANMessage &canMessage) {
            if (available()) {
                canMessage = messageInBuffer.front();
                messageInBuffer.erase(messageInBuffer.begin());
                return true;
            }

            return false;
        }

        int write(CANMessage &canMessage) {
            #ifdef ENABLE_ADVANCED_PRINT
            printf("[CAN]: CANMessage Write:\n");
            #endif
            printCANMessage(canMessage);

            if (writtenMsgCount < maxMsgInWriteBuffer) {
                writtenMsgCount++;
                sentMessages.push_back(canMessage);
                return 1;
            }

            #ifdef ENABLE_ADVANCED_PRINT
            printf("[CAN]: Write Aborted: Queue full\n");
            #endif
            return 0;
        }

        void clearWrite() {
            #ifdef ENABLE_ADVANCED_PRINT
            printf("[CAN]: Cleared Write Count\n");
            #endif
            writtenMsgCount = 0;
        }

        void clear() {
            messageInBuffer.clear();
            sentMessages.clear();
        }

        void addCustomIn(CANMessage &canMessage) {
            messageInBuffer.push_back(canMessage);
        }

        uint8_t RxIrq = 0;
    

        vector<CANMessage> messageInBuffer;
        vector<CANMessage> sentMessages;
        uint16_t maxMsgInWriteBuffer;
        uint16_t writtenMsgCount;
};


// Include class to test
#ifdef USE_MBED
    #define MBED_WAS_USED
    #undef USE_MBED
#endif

#include "communication/CCan.h"

#ifdef MBED_WAS_USED
    #define USE_MBED
    #undef MBED_WAS_USED
#endif

// Put needed things up
Sync syncer;
CCan canChannel(syncer, (PinName)1, (PinName)2, 3);

void getCarMessage(CANMessage &canMessage, CarMessage &carMessage) {
    carMessage.setIdsFromMessageHeader(canMessage.id);
    carMessage.setComponentId((id_component_t)canMessage.data[0]);

    carMessage.setLength(canMessage.len - 1);

    for (uint8_t i = 0; i < carMessage.getLength(); i++) {
        carMessage[i] = canMessage.data[i+1];
    }
}

void getCANMessage(CarMessage &carMessage, CANMessage &canMessage) {
    canMessage.format = CANStandard;
    canMessage.id = carMessage.getMessageHeader();
    canMessage.len = carMessage.getLength() + 1;
    canMessage.data[0] = carMessage.getComponentId();

    for (uint8_t i = 0; i < carMessage.getLength(); i++) {
        canMessage.data[i+1] = carMessage[i];
    }
}

// Actual Tests
CarMessage carMessage[TEST_MESSAGE_COUNT];

void testSending() {
    ((CAN*)canInstance)->maxMsgInWriteBuffer = TEST_MESSAGE_COUNT + 1;
    ((CAN*)canInstance)->clearWrite();

    // Send Messages
    for (uint16_t i = 0; i < TEST_MESSAGE_COUNT; i++) {
        canChannel.send(carMessage[i]);
    }

    // Check sent messages for equality
    for (uint16_t i = 0; i < TEST_MESSAGE_COUNT; i++) {
        if (i >= ((CAN*)canInstance)->sentMessages.size()) {
            #ifdef ENABLE_ADVANCED_PRINT
            printf("[testSending]: Tryed to access element %i from only %i sent elements", i, ((CAN*)canInstance)->sentMessages.size());
            #endif
            TEST_FAIL_MESSAGE("[testSending]: sent less Messages as expected!");
        }

        CarMessage currentReceivedMessage;
        getCarMessage(((CAN*)canInstance)->sentMessages[i], currentReceivedMessage);
        int comparisonResult = carMessage[i].compareTo(currentReceivedMessage);
        if (comparisonResult != 0) {
            #ifdef ENABLE_ADVANCED_PRINT
            printf("[testSending]: Failed at Msg Nr. %i with comparisonResult %i\n", i, comparisonResult);
            printf("Expected:\n");
            printCarMessage(carMessage[i]);
            printf("But got:\n");
            printCarMessage(currentReceivedMessage);
            #endif
            TEST_FAIL_MESSAGE("[testSending]: Messages not equal!");
        }
    }
}

void testReceiving() {
    ((CAN*)canInstance)->maxMsgInWriteBuffer = TEST_MESSAGE_COUNT + 1;
    ((CAN*)canInstance)->clearWrite();

    // Send Messages
    for (uint16_t i = 0; i < TEST_MESSAGE_COUNT; i++) {
        canChannel.send(carMessage[i]);
        CANMessage canMessage;
        getCANMessage(carMessage[i], canMessage);
        ((CAN*)canInstance)->addCustomIn(canMessage);
    }

    canChannel.run();

    // Check sent messages for equality
    for (uint16_t i = 0; i < TEST_MESSAGE_COUNT; i++) {
        if (i >= syncer.receivedCarMessages.size()) {
            #ifdef ENABLE_ADVANCED_PRINT
            printf("[testReceiving]: Tryed to access element %i from only %i received elements", i, syncer.receivedCarMessages.size());
            #endif
            TEST_FAIL_MESSAGE("[testReceived]: received less Messages as expected!");
        }

        int comparisonResult = carMessage[i].compareTo(syncer.receivedCarMessages[i]);
        if (comparisonResult != 0) {
            #ifdef ENABLE_ANDVANCED_PRINT
            printf("[testReceiving]: Failed at Msg Nr. %i with comparisonResult %i\n", i, comparisonResult);
            printf("Expected:\n");
            printCarMessage(carMessage[i]);
            printf("But got:\n");
            printCarMessage(syncer.receivedCarMessages[i]);
            #endif
            TEST_FAIL_MESSAGE("[testSending]: Messages not equal!");
        }
    }
}

void testSendingIfFull() {
    ((CAN*)canInstance)->maxMsgInWriteBuffer = 4;
    ((CAN*)canInstance)->clearWrite();

    // Send Messages
    for (uint16_t i = 0; i < TEST_MESSAGE_COUNT; i++) {
        canChannel.send(carMessage[i]);
    }

    while (canChannel.messageInQueue()) {
        ((CAN*)canInstance)->clearWrite();
        canChannel.run();
    }

    // Check sent messages for equality
    for (uint16_t i = 0; i < TEST_MESSAGE_COUNT; i++) {
        if (i >= ((CAN*)canInstance)->sentMessages.size()) {
            #ifdef ENABLE_ADVANCED_PRINT
            printf("[testSendingIfFull]: Tryed to access element %i from only %i sent elements", i, ((CAN*)canInstance)->sentMessages.size());
            #endif
            TEST_FAIL_MESSAGE("[testSendingIfFull]: sent less Messages as expected!");
        }

        CarMessage currentReceivedMessage;
        getCarMessage(((CAN*)canInstance)->sentMessages[i], currentReceivedMessage);
        int comparisonResult = carMessage[i].compareTo(currentReceivedMessage);
        if (comparisonResult != 0) {
            #ifdef ENABLE_ADVANCED_PRINT
            printf("[testSendingIfFull]: Failed at Msg Nr. %i with comparisonResult %i\n", i, comparisonResult);
            printf("Expected:\n");
            printCarMessage(carMessage[i]);
            printf("But got:\n");
            printCarMessage(currentReceivedMessage);
            #endif
            TEST_FAIL_MESSAGE("[testSendingIfFull]: Messages not equal!");
        }
    }
}

/*
uint8_t getHighestSendPriority(vector<CarMessage> &messageList) {
    uint8_t highestPriority = CAR_MESSAGE_PRIORITY_LOWEST;
    for (CarMessage &carMessage : messageList) {
        if (carMessage.getSendPriority() < highestPriority)
            highestPriority = carMessage.getSendPriority();
    }

    return highestPriority;
}

void testSendingWithPriority() {
    ((CAN*)canInstance)->maxMsgInWriteBuffer = 0;
    ((CAN*)canInstance)->clearWrite();

    vector<CarMessage> toBeSortedCarMessages;

    for (uint16_t i = 0; i < TEST_MESSAGE_COUNT; i++) {
        // Prepare Messages
        toBeSortedCarMessages.push_back(carMessage[i]);
        toBeSortedCarMessages[i].setSendPriority(i % 9);
        canChannel.send(toBeSortedCarMessages[i]);
    }

    ((CAN*)canInstance)->maxMsgInWriteBuffer = TEST_MESSAGE_COUNT + 1;
    canChannel.run();

    vector<CarMessage> sortedCarMessages;

    // Sort by highest Priority
    while(!toBeSortedCarMessages.empty()) {
        uint8_t highestSendPriority = getHighestSendPriority(toBeSortedCarMessages);
        for (auto carMessageIterator = toBeSortedCarMessages.begin(); carMessageIterator != toBeSortedCarMessages.end(); ) {
            if (carMessageIterator->getSendPriority() == highestSendPriority) {
                sortedCarMessages.push_back(*carMessageIterator);
                toBeSortedCarMessages.erase(carMessageIterator);
            } else {
                ++carMessageIterator;
            }
        }
    }

    // Check sent messages for equality
    for (uint16_t i = 0; i < TEST_MESSAGE_COUNT; i++) {
        if (i >= ((CAN*)canInstance)->sentMessages.size()) {
            #ifdef ENABLE_ADVANCED_PRINT
            printf("[testSendingWithPriority]: Tryed to access element %i from only %i sent elements", i, ((CAN*)canInstance)->sentMessages.size());
            #endif
            TEST_FAIL_MESSAGE("[testSendingWithPriority]: sent less Messages as expected!");
        }

        CarMessage currentReceivedMessage;
        getCarMessage(((CAN*)canInstance)->sentMessages[i], currentReceivedMessage);
        int comparisonResult = sortedCarMessages[i].compareTo(currentReceivedMessage);
        if (comparisonResult != 0) {
            #ifdef ENABLE_ADVANCED_PRINT
            printf("[testSendingWithPriority]: Failed at Msg Nr. %i with comparisonResult %i\n", i, comparisonResult);
            printf("Expected:\n");
            printCarMessage(sortedCarMessages[i]);
            printf("But got:\n");
            printCarMessage(currentReceivedMessage);
            #endif
            TEST_FAIL_MESSAGE("[testSendingWithPriority]: Messages not equal!");
        }
    }
}
*/

void testCCan() {
    UNITY_BEGIN();
    for (uint16_t i = 0; i < TEST_MESSAGE_COUNT; i++) {
        // Prepare Messages
        carMessage[i].setComponentId(i % 0x10);
        carMessage[i].setSenderId((id_device_t)(i % 0x20));
        carMessage[i].setReceiverId((id_device_t)(0x1F - (i % 0x20)));

        carMessage[i].setLength(i % 8);

        for (uint8_t j = 0; j < carMessage[i].getLength(); j++) {
            carMessage[i][j] = j + (i % 43); // because 42
        }
    }

    RUN_TEST(testSending);

    syncer.clear();
    ((CAN*)canInstance)->clear();

    RUN_TEST(testReceiving);

    syncer.clear();
    ((CAN*)canInstance)->clear();

    RUN_TEST(testSendingIfFull);

    /*
    syncer.clear();
    ((CAN*)canInstance)->clear();

    RUN_TEST(testSendingWithPriority);
    */

    UNITY_END();
}


void setup() {
    testCCan();
}