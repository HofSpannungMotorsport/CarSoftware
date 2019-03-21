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

#ifdef USE_ARDUINO
    void setup() {
        runtime.setup();
    }

    void loop() {
        runtime.loop();
    }
#endif // USE_ARDUINO