// Put your Software you want to test in here
// runtime.setup() gets called at the beginning,
// runtime.loop() gets called then over and over again...

#include "HardwarePwmBuzzerUnitTest.cpp"

class UnitTest {
    public:
        void setup() {

        }

        void loop() {
            HardwarePwmBuzzerUnitTest();
        }
};

UnitTest runtime;