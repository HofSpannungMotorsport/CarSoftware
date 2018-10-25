#ifndef IINTERRUPTBUTTON_H
#define IINTERRUPTBUTTON_H

#include <stdint.h>

enum button_state_t {
    STATE_PRESSED = true,
    STATE_NOT_PRESSED = false
};

enum button_type_t {
    NORMALLY_OPEN,
    NORMALLY_CLOSED
};

typedef uint16_t button_time_t;
typedef float button_debounce_time_t;

class IInterruptButton {
    public:
        virtual void setClickTime(button_time_t time) = 0;
        virtual void setLongClickTime(button_time_t time) = 0;
        virtual void setDebounceTime(button_debounce_time_t time) = 0;

        virtual button_state_t getPress() = 0;
        virtual button_state_t getClick() = 0;
        virtual button_state_t getLongClick() = 0;
        virtual button_state_t getLongClickStart() = 0;
};

#endif // IINTERRUPTBUTTON_H