// Put your Software you want to test in here
// runtime.setup() gets called at the beginning,
// runtime.loop() gets called then over and over again...

// Currently using RPM Unit Test

#include "CANFunctionalityTest.cpp"

class UnitTest {
    public:
        void setup() {
            CANFunctionalityTest();
        }

        void loop() {

        }
};

UnitTest runtime;