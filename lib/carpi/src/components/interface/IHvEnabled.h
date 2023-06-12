#ifndef IHV_ENABLED_H
#define IHV_ENABLED_H

#include "IComponent.h"

enum hv_enabled_on_state_t : bool {
    HV_ENABLED_ON_AT_HIGH = true,
    HV_ENABLED_ON_AT_LOW = false
};

class IHvEnabled : public IComponent {
    public:
        virtual bool read() = 0;
        virtual operator bool() = 0;
};

#endif // IHV_ENABLED_H