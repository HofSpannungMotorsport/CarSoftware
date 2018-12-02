// Put your Software you want to test in here
// runtime.setup() gets called at the beginning,
// runtime.loop() gets called then over and over again...

#include "AnalogRead.cpp"

class UnitTest {
    public:
        void setup() {
            AnalogRead();
        }

        void loop() {
            
        }
};

UnitTest runtime;