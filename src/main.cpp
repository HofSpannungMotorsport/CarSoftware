#ifdef TESTING_MODE
    #include "../hardwareTest/UnitTest.h"
#else
    #include "mbed.h"
    #include "car/Car.h"
#endif // TESTING_MODE

int main() {
    runtime.setup();

    while(1) {
        runtime.loop();
    }
}