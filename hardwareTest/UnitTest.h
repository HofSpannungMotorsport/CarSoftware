// Put your Software you want to test in here
// runtime.setup() gets called at the beginning,
// runtime.loop() gets called then over and over again...

#include "HardwarePedalUnitTest.cpp"

class UnitTest {
    public:
        void setup() {
            wait(2);
        }

        void loop() {
            HardwarePedalUnitTest();
        }
};

UnitTest runtime;