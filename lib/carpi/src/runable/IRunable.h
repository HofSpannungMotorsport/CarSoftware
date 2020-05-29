#ifndef IRUNABLE_H
#define IRUNABLE_H

#include "../components/interface/IComponent.h"

class IRunable : public IComponent {
    public:
        IRunable() {
            setObjectType(OBJECT_RUNABLE);
        }

        virtual void run() = 0;

        #if defined(USE_MBED) || defined(USE_ARDUINO) || defined(USE_TEENSYDUINO)
        virtual void run(Timer &timer) {
            timer.reset();
            timer.start();
            run();
            timer.stop();
        }
        #endif

        virtual void setStatus(status_t status) {
            _status = status;
        }

        virtual status_t getStatus() {
            return _status;
        }

    private:
        status_t _status = 0;
};

#endif // IRUNABLE_H