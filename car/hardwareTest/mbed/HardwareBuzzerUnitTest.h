#include "carpi.h"

#define BUZZER_PIN A5

HardwareBuzzer buzzer(BUZZER_PIN, BUZZER_ALARM);

void HardwareBuzzerUnitTest() {
    printf("BUZZER_MOMO_TONE\n");
    buzzer.setBeep(BUZZER_MONO_TONE);
    printf("BUZZER_ON\n");
    buzzer.setState(BUZZER_ON);
    wait(2);

    printf("BUZZER_BEEP_ON_BEEP_OFF\n");
    buzzer.setBeep(BUZZER_BEEP_ON_BEEP_OFF);
    wait(2);

    printf("BUZZER_BEEP_HIGH_BEEP_LOW\n");
    buzzer.setBeep(BUZZER_BEEP_HIGH_BEEP_LOW);
    wait(2);

    printf("BUZZER_BEEP_FAST_HIGH_LOW\n");
    buzzer.setBeep(BUZZER_BEEP_FAST_HIGH_LOW);
    wait(2);

    printf("BUZZER_OFF\n");
    buzzer.setState(BUZZER_OFF);
}