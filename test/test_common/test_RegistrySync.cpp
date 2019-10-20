#include <unity.h>

#define SYNC_H // Uninclude Sync
#include "runable/IRunable.h"
#include "communication/CarMessage.h"
#include "components/interface/ICommunication.h"

class Sync {
    public:
        Sync(ICommunication &localRegistry, ICommunication &distantRegistry)
        : _localRegistry(localRegistry), _distantRegistry(distantRegistry) {}

        void receive(CarMessage &carMessage) {

        }

        void send(CarMessage &carMessage) {
            if (carMessage.getComponentId() == _localRegistry.getComponentId()) {
                // -> message sent from local Registry
                _distantRegistry.receive(carMessage);
            } else {
                // -> message sent from distant Registry
                _localRegistry.receive(carMessage);
            }
        }

        virtual void run() {
            // do nothing....
        }

        virtual bool messageInQueue() {
            return false;
        }
    
    private:
        ICommunication &_localRegistry;
        ICommunication &_distantRegistry;
};

#include "components/internal/InternalRegistry.h"
#include "components/internal/InternalRegistryHardStorage.h"

InternalRegistry internalRegistry((id_sub_component_t)0x1);
InternalRegistry distantInternalRegistry((id_sub_component_t)0x2);
Sync syncer(internalRegistry, distantInternalRegistry);

uint8_t regNr = 0;
// returns true, if the registers are equal
template<typename T, typename registry_index_type_t>
bool compareRegister(registry_index_t size, T (IRegistry::*getPointer)(registry_index_type_t)) {
    regNr++;
    for (registry_index_t i = 0; i < size; i++) {
        T valueReg1 = (internalRegistry.*getPointer)((registry_index_type_t)i);
        T valueReg2 = (distantInternalRegistry.*getPointer)((registry_index_type_t)i);
        printf("Reg Nr. %i Value Nr. %i == Internal: %.5f Distant: %.5f", regNr, i, (float)valueReg1, (float)valueReg2);
        if (valueReg1 != valueReg2) {
            printf("Wrong Values!");
            return false;
        } else {
            printf("\n");
        }
    }

    return true;
}

void compareSyncedRegistries() {
    TEST_ASSERT_TRUE((compareRegister<float, float_registry_index_t>(float_registry_size, &IRegistry::getFloat)));
    TEST_ASSERT_TRUE((compareRegister<uint8_t, uint8_registry_index_t>(uint8_registry_size, &IRegistry::getUInt8)));
    TEST_ASSERT_TRUE((compareRegister<uint16_t, uint16_registry_index_t>(uint16_registry_size, &IRegistry::getUInt16)));
    TEST_ASSERT_TRUE((compareRegister<uint32_t, uint32_registry_index_t>(uint32_registry_size, &IRegistry::getUInt32)));
    TEST_ASSERT_TRUE((compareRegister<int8_t, int8_registry_index_t>(int8_registry_size, &IRegistry::getInt8)));
    TEST_ASSERT_TRUE((compareRegister<int16_t, int16_registry_index_t>(int16_registry_size, &IRegistry::getInt16)));
    TEST_ASSERT_TRUE((compareRegister<int32_t, int32_registry_index_t>(int32_registry_size, &IRegistry::getInt32)));
    TEST_ASSERT_TRUE((compareRegister<bool, bool_registry_index_t>(bool_registry_size, &IRegistry::getBool)));
}

void registrySyncTest() {
    internalRegistry.attach(syncer);
    distantInternalRegistry.attach(syncer);    

    InternalRegistryHardStorage::loadIn(internalRegistry, &syncer);
    distantInternalRegistry.setFloat(ALIVE_SIGNAL_REFRESH_RATE, distantInternalRegistry.getFloat(ALIVE_SIGNAL_REFRESH_RATE) + (float)69.69);

    UNITY_BEGIN();
    RUN_TEST(compareSyncedRegistries);
    UNITY_END();
}

#if defined(USE_ARDUINO) || defined(USE_TEENSYDUINO)
#include <Arduino.h>

void setup() {
    // NOTE!!! Wait for >2 secs
    // if board doesn't support software reset via Serial.DTR/RTS
    delay(2000);

    registrySyncTest();
}

void loop() {
    digitalWrite(13, HIGH);
    delay(100);
    digitalWrite(13, LOW);
    delay(500);
}

#else

int main() {
    registrySyncTest();

    return 0;
}

#endif