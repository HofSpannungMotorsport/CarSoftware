// Put your Software you want to test in here
// runtime.setup() gets called at the beginning,
// runtime.loop() gets called then over and over again...

#ifdef USE_MBED
    // Include Framework
    #include "mbed.h"
    #ifndef MESSAGE_REPORT
        #define MESSAGE_REPORT
        Serial pcSerial(USBTX, USBRX); // Connection to PC over Serial
    #endif
#endif // USE_MBED

#ifdef USE_ARDUINO
    // Include Framework
    #include "Arduino.h"
    HardwareSerial &pcSerial = Serial;
#endif // USE_ARDUINO


// Include the test program here
#include ".h"

class UnitTest {
    public:
        void setup() {
            wait(0.1);
        }

        void loop() {
            // Main Function here, it will be called constantly
            
        }
};

UnitTest runtime;