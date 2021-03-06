#ifndef IHV_ENABLED_H
#define IHV_ENABLED_H

#include "IComponent.h"

class IHvEnabled : public IComponent {
    public:
        virtual bool read() = 0;
        virtual operator bool() = 0;
};

#endif // IHV_ENABLED_H