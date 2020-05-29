#pragma once

#ifndef DISABLE_UNITY
#include <unity.h>
#endif

#include "vector"

#define STEROIDO_DISABLE_LOOP

#include "../steroido/src/Steroido.h"
#include "../steroido/src/Common/ITimer.h"


#define SYNC_USE_STACK_VECTOR

#ifdef SYNC_USE_STACK_VECTOR
    #define SYNC_MAX_DEVICES_COUNT 3
    #define SYNC_MAX_CHANNELS_COUNT 3
    #define SYNC_MAX_COMPONENTS_COUNT 5
#endif

unsigned long globalMillis = 0;


class Timer : public ITimer {
    protected:
        unsigned long getMillis() {
            return globalMillis;
        }
};

#include "../communication/Sync.h"
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



/**
 * @brief Dummy Component for Testing Sync
 * 
 */
class ITestComponent : public SelfSyncable {
    public:
        ITestComponent() {
            setComponentType((id_component_type_t)1);
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

/**
 * @brief Create a more or less random CarMessage. Only Data gets filled randomly
 * 
 * @param seed the Seed the randomness should be based on
 * @return CarMessage 
 */
CarMessage createRandomMessage(uint16_t seed) {
    CarMessage carMessage;

    carMessage.setLength(seed % STD_CARMESSAGE_DATA_SIZE + 1);

    for (uint8_t i = 0; i < carMessage.getLength(); i++) {
        carMessage.set(i * i * (uint8_t)seed, i);
    }

    return carMessage;
}

/**
 * @brief Create some example/random messages
 * 
 * @param _exampleMessages 
 * @param count 
 * @param customStartSeed 
 */
void createExampleMessages(std::vector<CarMessage> &_exampleMessages, uint16_t count, uint16_t customStartSeed = 1) {
    for (uint16_t i = 0; i < count; i++) {
        uint16_t currentSeed = i + customStartSeed;
        _exampleMessages.push_back(createRandomMessage(currentSeed));
    }
}



Sync pedalSync(DEVICE_PEDAL);
Sync dashboardSync(DEVICE_DASHBOARD);
Sync masterSync(DEVICE_MASTER);

TestNetworkNode pedalChannel(pedalSync, CHANNEL_PEDAL);
TestNetworkNode dashboardChannel(dashboardSync, CHANNEL_DASHBOARD);
TestNetworkNode masterChannel(masterSync, CHANNEL_MASTER);

HardwareTestComponent hardwareTestComponent((id_sub_component_t)2);
SoftwareTestComponent softwareTestComponent((id_sub_component_t)2);

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

        if (unsyncedMessagesCount >= TEST_MESSAGES_CLUSTER_SIZE) {
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
    printf("Begin with SimpoleFloodSyncing\n");

    for (CarMessage &carMessage : exampleMessages) {
        if (!hardwareTestComponent.syntheticSend(carMessage)) {
            printf("Buffer full, flooding now\n");
            pedalSync.run();
            masterSync.run();

            if (!hardwareTestComponent.syntheticSend(carMessage))
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

    createExampleMessages(exampleMessages, TEST_MESSAGES_COUNT);

    printf("Prepared Example Messages\n");

    #ifdef DISABLE_UNITY

    testSimpleStableSyncing();
    softwareTestComponent.receivedMessages.clear();
    testSimpleFloodSyncing();

    #else // DISABLE_UNITY

    UNITY_BEGIN();
    RUN_TEST(testSimpleStableSyncing);
    softwareTestComponent.receivedMessages.clear();
    RUN_TEST(testSimpleFloodSyncing);
    UNITY_END();

    #endif // DISABLE_UNITY
}