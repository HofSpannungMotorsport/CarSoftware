#ifndef ISHUTDOWN_H
#define ISHUTDOWN_H

#include "IComponent.h"

class IShutdown : public IComponent {
    public:
        virtual bool read() = 0;
        virtual operator bool() = 0;
};

#endif // ISHUTDOWN_H