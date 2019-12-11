#ifdef TESTING_MODE
    #include "hardwareTest/UnitTest.h"
#else
    #include "Car.h"
#endif // TESTING_MODE

void setup() {
    runtime.setup();
}

void loop() {
    #ifdef STEROIDO_SCHEDULER_RUN_NEEDED
        scheduler.run();
    #endif

    runtime.loop();
}