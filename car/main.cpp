#ifdef TESTING_MODE
    #include "hardwareTest/UnitTest.h"
#else
    #include "Car.h"
#endif // TESTING_MODE

#ifdef USE_MBED
    int main() {
        runtime.setup();

        while(1) {
            runtime.loop();
        }
    }
#endif // USE_MBED

#if defined(USE_ARDUINO) || defined(USE_TEENSYDUINO)
    void setup() {
        runtime.setup();
    }

    void loop() {
        runtime.loop();
    }
#endif // USE_ARDUINO || USE_TEENSYDUINO