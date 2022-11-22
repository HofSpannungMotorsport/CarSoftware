#ifndef IDISPLAY_H
#define IDISPLAY_H

#include "ICommunication.h"

typedef uint8_t display_status_t;

class IDisplay : public ICommunication {
    public:

        virtual display_status_t getStatus() = 0;
        //virtual pedal_value_t getValue() = 0;

        //virtual void setStatus(pedal_status_t status) = 0;
        //virtual void setValue(pedal_value_t value) = 0;
};

#endif