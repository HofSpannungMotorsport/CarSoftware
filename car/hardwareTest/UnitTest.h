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
    #define pcSerial Serial
#endif // USE_ARDUINO

#ifdef USE_TEENSYDUINO
    // Include Framework
    #include "Arduino.h"
    #define pcSerial Serial
#endif // USE_TEENSYDUINO

// Include the test program here
#include "mbed/HardwareSDCardTest.h"

class UnitTest {
    public:
        void setup() {
            #ifdef USE_MBED
                wait(0.1);
            #endif

            // Setup Function here, it will be called once at startup
            
        }

        void loop() {
            // Loop Function here, it will be called constantly
            HardwareSDCardTest();
        }
};

UnitTest runtime;