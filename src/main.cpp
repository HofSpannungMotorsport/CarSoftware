#include "mbed.h"
#include "car/Car.h"

int main() {
    runtime.setup();

    while(1) {
        runtime.loop();
    }
}