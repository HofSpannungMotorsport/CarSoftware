// Put your Software you want to test in here
// runtime.setup() gets called at the beginning,
// runtime.loop() gets called then over and over again...

#ifndef MESSAGE_REPORT
    #define MESSAGE_REPORT
    Serial pcSerial(USBTX, USBRX); // Connection to PC over Serial
#endif


// Include the test program here
#include ".cpp"

class UnitTest {
    public:
        void setup() {
            wait(2);
        }

        void loop() {
            // Main Function here, it will be called constantly
            
        }
};

UnitTest runtime;