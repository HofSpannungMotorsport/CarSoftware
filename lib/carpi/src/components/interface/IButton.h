#ifndef IINTERRUPTBUTTON_H
#define IINTERRUPTBUTTON_H

#include "communication/SelfSyncable.h"
#include "HardConfig.h"

enum button_message_command_t : uint8_t {
    BUTTON_MESSAGE_COMMAND_ADD_STATE = 0x0
};

enum button_error_type_t : status_t {
    UNDEFINED_BUTTON_ERROR =        0x1,
    STATE_BUFFER_FULL =             0x2,
    WRONG_STATE =                   0x4,
    ADDED_SAME_STATE_AS_LAST_TIME = 0x8
};

enum button_state_t : uint8_t {
    NOT_PRESSED = 0,
    PRESSED,
    LONG_CLICKED,
    WRONG
};

enum button_type_t {
    NORMALLY_OPEN,
    NORMALLY_CLOSED
};

typedef uint16_t button_time_t;
typedef float button_debounce_time_t;

/*
    To get the current State of a button, use getState() to get the current
    state and use getStateChanged() if the State has changed.
*/

class IButton : public SelfSyncable {
    public:
        IButton() {
            setComponentType(COMPONENT_BUTTON);
        }

        // Return or set the current State of the button
        // Calculated throu the methods below
        virtual void setState(button_state_t state) = 0;
        virtual button_state_t getState() = 0;

        // See if the State has Changed since the last call of getState();
        virtual bool getStateChanged() = 0;
};

#endif // IINTERRUPTBUTTON_H
