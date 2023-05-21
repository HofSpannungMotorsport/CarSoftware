#pragma once
#include "Mutex.h"

class SharedMutex
{
public:
    SharedMutex() : mutex() {}

    void lock()
    {
        mutex.lock();
    }

    void unlock()
    {
        mutex.unlock();
    }

private:
    Mutex mutex;
};