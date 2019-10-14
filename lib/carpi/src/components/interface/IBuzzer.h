#ifndef IBUZZER_H
#define IBUZZER_H

#include "IComponent.h"

enum buzzer_state_t : bool {
    BUZZER_OFF = false,
    BUZZER_ON  = true
};

enum buzzer_beep_type_t : uint8_t {
    BUZZER_MONO_TONE = 0x0,
    BUZZER_BEEP_ON_BEEP_OFF = 0x1,
    BUZZER_BEEP_HIGH_BEEP_LOW = 0x2,
    BUZZER_BEEP_FAST_HIGH_LOW = 0x3
};

typedef uint16_t buzzer_hz_t;

enum buzzer_error_type_t : status_t {
    BUZZER_OK = 0x0,
    BUZZER_ERROR_UNKNOWN = 0x1,
    BUZZER_WRONG_STATE = 0x2,
    BUZZER_WRONG_BEEP = 0x4
};

class IBuzzer : public IComponent {
    public:
        IBuzzer() {
            setComponentType(COMPONENT_BUZZER);
        }

        virtual void setState(buzzer_state_t state) = 0;
        virtual buzzer_state_t getState() = 0;

        virtual void setBeep(buzzer_beep_type_t beep) = 0;
        virtual buzzer_beep_type_t getBeep() = 0;

        virtual void setHz(buzzer_hz_t hz) = 0;
        virtual buzzer_hz_t getHz() = 0;
};

#endif // IBUZZER_H