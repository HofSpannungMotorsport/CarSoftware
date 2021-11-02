#pragma once

#ifndef DISABLE_UNITY
#include <unity.h>
#endif

#include "vector"

#define STEROIDO_DISABLE_LOOP

#include "../steroido/src/Steroido.h"
#include "../steroido/src/Common/ITimer.h"

// #define SYNC_USE_STACK_VECTOR
#ifdef SYNC_USE_STACK_VECTOR
    #define SYNC_MAX_DEVICES_COUNT 3
    #define SYNC_MAX_CHANNELS_COUNT 3
    #define SYNC_MAX_COMPONENTS_COUNT 5
#endif

#define SYNC_DEBUG_IN_BUFFER_DROPPED_MESSAGES

#include "TestingTimer.h"
#include "../communication/Sync/Sync.h"
#include "../communication/IChannel.h"
#include "../communication/SelfSyncable.h"
#include "../communication/CarMessage.h"


enum ChannelId : uint8_t {
    CHANNEL_MASTER = 1,
    CHANNEL_PEDAL,
    CHANNEL_DASHBOARD
};

class IVirtualChannel : public IChannel {
    public:
        virtual void physicallyGot(CarMessage &carMessage) = 0;
};

struct ChannelIdPair {
    IVirtualChannel* channel;
    ChannelId channelId;
};

std::vector<ChannelIdPair> virtualNetwork;

void sendToAllExceptOf(CarMessage &carMessage, ChannelId channelId) {
    for (ChannelIdPair &channelIdPair : virtualNetwork) {
        if (channelIdPair.channelId != channelId) {
            channelIdPair.channel->physicallyGot(carMessage);
        }
    }
}


/**
 * @brief Dummy Channel for Testing Sync
 * 
 */
class TestNetworkNode : public IVirtualChannel {
    public:
        TestNetworkNode(Sync &syncer, ChannelId channelId) : _syncer(syncer), _channelId(channelId) {
            ChannelIdPair thisPair;
            thisPair.channel = this;
            thisPair.channelId = channelId;

            virtualNetwork.push_back(thisPair);
        }

        bool send(CarMessage &carMessage) {
            sendToAllExceptOf(carMessage, _channelId);
            return true;
        }

        void physicallyGot(CarMessage &carMessage) {
            _syncer.receive(carMessage);
        }

        void setStatus(status_t status) {} // No implementation needed

        status_t getStatus() {
            return 0;
        }

        void run() {} // No implementation needed

    private:
        Sync& _syncer;
        ChannelId _channelId;
};

template <uint16_t bufferSize>
class EmptyChannel : public IChannel {
    public:
        EmptyChannel() {}

        bool send(CarMessage &carMessage) {
            _buffer.push(carMessage);
            return true;
        }

        bool pop(CarMessage &carMessage) {
            return _buffer.pop(carMessage);
        }


        void setStatus(status_t status) {} // No implementation needed

        status_t getStatus() {
            return 0;
        }

        void run() {} // No implementation needed
    
    private:
        CircularBuffer<CarMessage, bufferSize> _buffer;
};



/**
 * @brief Dummy Component for Testing Sync
 * 
 */
class ITestComponent : public SelfSyncable {
    public:
        ITestComponent() {
            setComponentType(COMPONENT_PEDAL);
        }

        void setStatus(status_t status) {} // No implementation needed

        status_t getStatus() {
            return 0;
        }

        void receive(CarMessage &carMessage) {
            receivedMessages.push_back(carMessage);
        }

        bool syntheticSend(CarMessage &carMessage) {
            return _send(carMessage);
        }
    
        std::vector<CarMessage> receivedMessages;
};

class SoftwareTestComponent : public ITestComponent {
    public:
        SoftwareTestComponent(id_sub_component_t componentSubId) {
            setObjectType(OBJECT_SOFTWARE);
            setComponentSubId(componentSubId);
        }
};

class HardwareTestComponent : public ITestComponent {
    public:
        HardwareTestComponent(id_sub_component_t componentSubId) {
            setObjectType(OBJECT_HARDWARE);
            setComponentSubId(componentSubId);
        }
};



/**
 * @brief Compares two Vectors (CarMessage)
 * 
 * @param buffer1 Reference to the first Buffer/Vector
 * @param buffer2 
 * @return uint8_t 0 = Equal, 1 = different size, 2 = different Element(s)
 */
uint8_t compareBuffers(std::vector<CarMessage> &buffer1, std::vector<CarMessage> &buffer2) {
    if (buffer1.size() != buffer2.size()) {
        printf("[compareBuffers]: buffer1-size: %i\tbuffer2-size: %i\n", buffer1.size(), buffer2.size());
        return 1;
    } 
    
    for (uint16_t i = 0; i < buffer1.size(); i++) {
        int compareResult = buffer1.at(i).compareTo(buffer2.at(i));

        if (compareResult != 0)
            return 2;
    }

    return 0;
}


void createNumberedMessages(std::vector<CarMessage>& _messages, uint16_t count, uint16_t startNumber, id_component_t componentId) {
    for (uint16_t i = 0; i < count; ++i) {
        CarMessage carMessage;
        carMessage.setLength(1);
        carMessage.set(i, 0);
        carMessage.setComponentId(componentId);
        _messages.push_back(carMessage);
    }
}



// ------------------------------------------- SyncedOutBuffer -------------------------------------------

std::vector<CarMessage> syncedOutBufferMessages;

bool testSyncedOutBuffer() {
    printf("Begin with SyncedOutBuffer\n");


    // ############################################

    // First, test standard "Sending"

    // ############################################


    bool success = true;
    SyncedOutBuffer syncedOutBuffer;

    // Create some messages
    
    createNumberedMessages(syncedOutBufferMessages, STD_SYNCED_BUFFER_MESSAGES_BUFFER_SIZE * 3, 0, componentId::getComponentId(COMPONENT_PEDAL, COMPONENT_PEDAL_GAS));

    uint16_t count = STD_SYNCED_BUFFER_MESSAGES_BUFFER_SIZE + 1;
    uint16_t index = 0;
    while(count--) {
        syncedOutBufferMessages[index].setComponentId(componentId::getComponentId(COMPONENT_PEDAL, COMPONENT_PEDAL_GAS));
        syncedOutBufferMessages[index].setSenderId(DEVICE_PEDAL);
        syncedOutBufferMessages[index].setReceiverId(DEVICE_MASTER);

        if (!syncedOutBuffer.add(syncedOutBufferMessages[index++])) {
            if (index != STD_SYNCED_BUFFER_MESSAGES_BUFFER_SIZE + 1) {
                printf("@testSyncedOutBuffer: Can't push message to buffer. index: %i\n", index);
                success = false;
            }

            break;
        }

        if (index > STD_SYNCED_BUFFER_MESSAGES_BUFFER_SIZE) {
            printf("@testSyncedOutBuffer: pushed more than possbile messages!\n");
            success = false;
        }
    }

    // -> Buffer filled completely
    // Now empty it by sending

    printf("Filled up -> now emptying\n");

    EmptyChannel<STD_SYNCED_BUFFER_MESSAGES_BUFFER_SIZE * 10> loopbackChannel;

    while (syncedOutBuffer.sendNext(loopbackChannel)) {}

    CarMessage newestMessage;
    uint16_t poppedMessagesCount = 0;

    uint16_t reconstructedIndex = 0;
    while (true) {
        CarMessage poppedMessage;
        if (loopbackChannel.pop(poppedMessage)) {
            newestMessage = poppedMessage;
            poppedMessagesCount++;

            if (poppedMessage.get(0) != reconstructedIndex++) {
                printf("@testSynceedOutBuffer: Wrong Message Number!\n");
            }
        } else {
            break;
        }
    }

    if (syncedOutBuffer.add(syncedOutBufferMessages[index-1])) {
        printf("@testSyncedOutBuffer: Can push without confirming before!\n");
        success = false;
    }

    syncedOutBuffer.confirm(newestMessage.getMessageId());
    if (poppedMessagesCount != STD_SYNCED_BUFFER_MESSAGES_BUFFER_SIZE) {
        printf("@testSyncedOutBuffer: Wrong amount of Messages sent over channel!\n");
        success = false;
    }

    count = STD_SYNCED_BUFFER_MESSAGES_BUFFER_SIZE + 1;
    index -= 1;
    while(count--) {
        syncedOutBufferMessages[index].setComponentId(componentId::getComponentId(COMPONENT_PEDAL, COMPONENT_PEDAL_GAS));
        syncedOutBufferMessages[index].setSenderId(DEVICE_PEDAL);
        syncedOutBufferMessages[index].setReceiverId(DEVICE_MASTER);

        if (!syncedOutBuffer.add(syncedOutBufferMessages[index++])) {
            if (index != STD_SYNCED_BUFFER_MESSAGES_BUFFER_SIZE * 2 + 1) {
                printf("@testSyncedOutBuffer: Can't push message to buffer. index: %i (2)\n", index);
                success = false;
            }

            break;
        }

        if (index > STD_SYNCED_BUFFER_MESSAGES_BUFFER_SIZE * 2) {
            printf("@testSyncedOutBuffer: pushed more than possbile messages! (2)\n");
            success = false;
        }
    }

    // -> Pushing, confirming and re-pushing correct

    if (success) {
        printf("SyncedOutBuffer Test Success!\n");
    } else {
        printf("SyncedOutBuffer Test Error!\n");
    }

    return success;
}





// ------------------------------------------- Sync -------------------------------------------

Sync pedalSync(DEVICE_PEDAL);
Sync dashboardSync(DEVICE_DASHBOARD);
Sync masterSync(DEVICE_MASTER);

TestNetworkNode pedalChannel(pedalSync, CHANNEL_PEDAL);
TestNetworkNode dashboardChannel(dashboardSync, CHANNEL_DASHBOARD);
TestNetworkNode masterChannel(masterSync, CHANNEL_MASTER);

HardwareTestComponent hardwareTestComponent(COMPONENT_PEDAL_GAS);
SoftwareTestComponent softwareTestComponent(COMPONENT_PEDAL_GAS);

std::vector<CarMessage> exampleMessages;

void createRoutingTable() {
    // Pedal Routing
    pedalSync.addDevice(pedalChannel, DEVICE_DASHBOARD);
    pedalSync.addDevice(pedalChannel, DEVICE_MASTER);

    // Dashboard Routing
    dashboardSync.addDevice(dashboardChannel, DEVICE_PEDAL);
    dashboardSync.addDevice(dashboardChannel, DEVICE_MASTER);

    // Master Routing
    masterSync.addDevice(masterChannel, DEVICE_PEDAL);
    masterSync.addDevice(masterChannel, DEVICE_DASHBOARD);
}

void prepareComponents() {
    // TestComponent
    pedalSync.addComponent(hardwareTestComponent, DEVICE_MASTER);
    masterSync.addComponent(softwareTestComponent, DEVICE_PEDAL);

    hardwareTestComponent.attach(pedalSync);
    softwareTestComponent.attach(masterSync);
}



#define TEST_MESSAGES_COUNT 200
#define TEST_MESSAGES_CLUSTER_SIZE 5

/**
 * @brief Test for sending some messages over the Syncer without hardware errors
 * 
 */
void testSimpleStableSyncing() {
    printf("Begin with SimpleStableSyncing\n");

    uint16_t unsyncedMessagesCount = 0;
    for (CarMessage &carMessage : exampleMessages) {
        if (!hardwareTestComponent.syntheticSend(carMessage)) {
            printf("[testSimpleStableSync]: Sending not possible!\n");
        } else {
            unsyncedMessagesCount++;
        }

        ++globalMillis;

        if (unsyncedMessagesCount >= TEST_MESSAGES_CLUSTER_SIZE) {
            globalMillis += STD_SYNC_INTERVAL_MESSAGE_TIME;
            pedalSync.run();
            masterSync.run();
            unsyncedMessagesCount = 0;
        }
    }

    pedalSync.run();
    masterSync.run();

    #ifdef DISABLE_UNITY
    uint16_t compareResult = compareBuffers(exampleMessages, softwareTestComponent.receivedMessages);
    if (compareResult == 0) {
        printf("[testSimpleStableSyncing]: Test Success!\n");
    } else {
        printf("[testSimpleStableSyncing]: Error running Test! Expected 0, Got %i\n", compareResult);
    }
    #else // DISABLE_UNITY
    TEST_ASSERT_EQUAL(0, compareBuffers(exampleMessages, softwareTestComponent.receivedMessages));
    #endif
}

/**
 * @brief Test Sync until full, then try to flood out all hanging massages
 * 
 */
void testSimpleFloodSyncing() {
    printf("Begin with SimpleFloodSyncing\n");

    for (CarMessage &carMessage : exampleMessages) {
        globalMillis++;
        if (!hardwareTestComponent.syntheticSend(carMessage)) {
            printf("[testSimpleFloodSyncing]: Still cant't send out new Message!\n");
        }
    }

    pedalSync.run();
    masterSync.run();

    #ifdef DISABLE_UNITY
    uint16_t compareResult = compareBuffers(exampleMessages, softwareTestComponent.receivedMessages);
    if (compareResult == 0) {
        printf("[testSimpleFloodSyncing]: Test Success!\n");
    } else {
        printf("[testSimpleFloodSyncing]: Error running Test! Expected 0, Got %i\n", compareResult);
    }
    #else // DISABLE_UNITY
    TEST_ASSERT_EQUAL(0, compareBuffers(exampleMessages, softwareTestComponent.receivedMessages));
    #endif
}



void setup() {
    printf("Start\n");

    createRoutingTable();
    prepareComponents();

    printf("Created Network\n");

    createNumberedMessages(exampleMessages, TEST_MESSAGES_COUNT, 0, componentId::getComponentId(COMPONENT_PEDAL, COMPONENT_PEDAL_GAS));

    printf("Prepared Example Messages\n");

    #ifdef DISABLE_UNITY

    //testSyncedOutBuffer();
    testSimpleStableSyncing();
    softwareTestComponent.receivedMessages.clear();
    testSimpleFloodSyncing();

    #else // DISABLE_UNITY

    UNITY_BEGIN();
    RUN_TEST(testSyncedOutBuffer);
    RUN_TEST(testSimpleStableSyncing);
    softwareTestComponent.receivedMessages.clear();
    RUN_TEST(testSimpleFloodSyncing);
    UNITY_END();

    #endif // DISABLE_UNITY
}