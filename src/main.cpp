#include "mbed.h"
#include "car/Car.h"

#ifdef TESTING_MODE
#include "../test/UnitTest.h"
#endif // TESTING_MODE

int main() {
    runtime.setup();

    while(1) {
        runtime.loop();
    }
}