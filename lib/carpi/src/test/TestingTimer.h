#pragma once

#include "../steroido/src/Common/ITimer.h"

unsigned long globalMillis = 0;

class Timer : public ITimer {
    protected:
        unsigned long getMillis() {
            return globalMillis;
        }
};