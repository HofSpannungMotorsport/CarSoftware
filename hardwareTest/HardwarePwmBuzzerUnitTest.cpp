#include "../src/components/hardware/HardwarePwmBuzzer.h"
#include "../src/can/can_ids.h"

#define BUZZER_PIN A2

HardwarePwmBuzzer buzzer(BUZZER_PIN, BUZZER_ALARM);

void HardwarePwmBuzzerUnitTest() {
    buzzer.setBeep(BUZZER_MONO_TONE);
    buzzer.setState(BUZZER_ON);
    wait(2);

    buzzer.setBeep(BUZZER_BEEP_ON_BEEP_OFF);
    wait(2);

    buzzer.setBeep(BUZZER_BEEP_HIGH_BEEP_LOW);
    wait(2);

    buzzer.setBeep(BUZZER_BEEP_FAST_HIGH_LOW);
    wait(2);

    buzzer.setState(BUZZER_OFF);
}