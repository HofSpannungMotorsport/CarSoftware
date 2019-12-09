#include <unity.h>
#ifdef TRAVIS
    #include "crossplatform/memcpy.h"
#endif

#ifdef USE_MBED
    #include "mbed.h"
#else
    #if defined(USE_ARDUINO) || defined(USE_TEENSYDUINO)
        #define pcSerial Serial
    #endif
    #include "crossplatform/CircularBuffer.h"
#endif

#include "communication/Sync.h"
#include "communication/IChannel.h"
#include "communication/SelfSyncable.h"

class TestChannel : public IChannel {
    public:
        TestChannel(Sync &syncer) : _syncer(syncer) {}

        void send(CarMessage &carMessage) {
            // loop back
            _syncer.receive(carMessage);
        }

        bool messageInQueue() {
            return _messageInQueue;
        }

        void giveToSyncer(CarMessage &carMessage) {
            _syncer.receive(carMessage);
        }

        void run() {}

        Sync &_syncer;
        bool _messageInQueue = false;
};

class TestComponent : public SelfSyncable {
    public:
        TestComponent(id_sub_component_t componentSubId) {
            setComponentType(COMPONENT_SYSTEM);
            setObjectType(OBJECT_INTERNAL);
            setComponentSubId(componentSubId);
        }

        void setStatus(status_t status) {}
        status_t getStatus() {return 0;}

        void receive(CarMessage &carMessage) {
            _receivedMessages.push_back(carMessage);
        }

        void synteticSend(CarMessage &carMessage) {
            if (_syncerAttached) {
                _syncer->send(carMessage);
            }
        }

        void synteticSendCommand(uint8_t command) {
            _sendCommand(command, IS_NOT_DROPABLE);
        }

        void synteticSendCommand(uint8_t command, uint8_t value) {
            _sendCommand(command, value, IS_NOT_DROPABLE);
        }

        void synteticSendCommand(uint8_t command, uint8_t values[], uint8_t valueAmount) {
            _sendCommand(command, values, valueAmount, IS_NOT_DROPABLE);
        }

        vector<CarMessage> _receivedMessages;
};


Sync sync(DEVICE_MASTER);
TestChannel testChannel(sync);
TestComponent testComponent((id_sub_component_t)0x6);


void testComponentSync() {
    sync.addComponent(testComponent, testChannel, DEVICE_MASTER);
    testComponent.attach(sync);

    // Send with command sending
    testComponent.synteticSendCommand(69);
    testComponent.synteticSendCommand(66, 99);
    uint8_t testValues[] = {8, 9, 10, 11, 12};
    testComponent.synteticSendCommand(33, testValues, sizeof(testValues) / sizeof(uint8_t));

    sync.run();

    // Check if received Messages are right
    if (testComponent._receivedMessages.size() != 3) {
        TEST_FAIL_MESSAGE("[testComponentSync]: Not Received enough messages!");
    }

    // Get submessage, then check data
    car_sub_message_t currentSubMessage;
    CarMessage &firstMessage = testComponent._receivedMessages[0];
    currentSubMessage = firstMessage.subMessages[0];
    if (currentSubMessage.data[0] != 69) {
        TEST_FAIL_MESSAGE("[testComponentSync]: First Message not received correctly");
    }

    CarMessage &secondMessage = testComponent._receivedMessages[1];
    currentSubMessage = secondMessage.subMessages[0];
    if (currentSubMessage.data[0] != 66 ||
        currentSubMessage.data[1] != 99) {
        TEST_FAIL_MESSAGE("[testComponentSync]: Second Message not received correctly");
    }

    CarMessage &thirdMessage = testComponent._receivedMessages[2];
    currentSubMessage = thirdMessage.subMessages[0];
    if (currentSubMessage.data[0] != 33 ||
        currentSubMessage.data[1] != 8 ||
        currentSubMessage.data[2] != 9 ||
        currentSubMessage.data[3] != 10 ||
        currentSubMessage.data[4] != 11 ||
        currentSubMessage.data[5] != 12) {
        TEST_FAIL_MESSAGE("[testComponentSync]: Third Message not received correctly");
    }
}

class TestSourceChannel : public IChannel {
    public:
        TestSourceChannel(Sync &sync) : _sync(sync) {}

        void synteticSend(CarMessage &carMessage) {
            _sync.receive(carMessage);
        }

        void send(CarMessage &carMessage) {}

        void run() {}

        bool messageInQueue() {
            return false;
        }

        void setStatus(status_t status) {}
        status_t getStatus() {
            return 0;
        }

        Sync &_sync;
};

class TestDestinationChannel : public IChannel {
    public:
        TestDestinationChannel(Sync &sync) : _sync(sync) {}

        void send(CarMessage &carMessage) {
            _receivedMessages.push_back(carMessage);
        }

        void run() {}

        bool messageInQueue() {
            return false;
        }

        void setStatus(status_t status) {}
        status_t getStatus() {
            return 0;
        }

        Sync &_sync;
        vector<CarMessage> _receivedMessages;
};

TestSourceChannel sourceChannel(sync);
TestDestinationChannel destChannel(sync);

void testBridge() {
    sync.addBridge(0x12, sourceChannel, destChannel, DEVICE_ALL, DEVICE_DASHBOARD);

    CarMessage carMessage;

    car_sub_message_t subMessage;
    subMessage.length = 1;
    subMessage.data[0] = 69;

    carMessage.addSubMessage(subMessage);
    carMessage.setSenderId(DEVICE_PEDAL);
    carMessage.setReceiverId(DEVICE_DASHBOARD);
    carMessage.setComponentId(0x12);
    sourceChannel.synteticSend(carMessage);

    sync.run();

    if (destChannel._receivedMessages.size() != 1) {
        TEST_FAIL_MESSAGE("[testBridge]: no message received");
    }

    CarMessage &carMessageReceived = destChannel._receivedMessages[0];
    if (carMessageReceived.subMessages.size() != 1) {
        TEST_FAIL_MESSAGE("[testBridge]: No subMessage attached");
    }

    car_sub_message_t &subMessageReceived = carMessageReceived.subMessages[0];
    if (subMessageReceived.length != 1) {
        TEST_FAIL_MESSAGE("[testBridge]: Wrong subMessage Length");
    }

    if (subMessageReceived.data[0] != 69) {
        TEST_FAIL_MESSAGE("[testBrdige]: Wrong subMessage content");
    }
}

void testSync() {
    UNITY_BEGIN();
    RUN_TEST(testComponentSync);
    RUN_TEST(testBridge);
    UNITY_END();
}


int main() {
    testSync();
    return 0;
}