#define USE_NATIVE
#define DESKTOP_NATIVE
#define NATIVE


#ifdef DESKTOP_NATIVE


#define DISABLE_UNITY
#define MESSAGE_REPORT
#include "../lib/carpi/src/test/test_Sync.h"


#else // DESKTOP_NATIVE


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


#endif // DESKTOP_NATIVE