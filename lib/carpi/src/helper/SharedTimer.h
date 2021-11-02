#ifndef DYNAMIC_TIMER_H
#define DYNAMIC_TIMER_H

/**
 * @brief Basic Wrapper of the Timer to be able to copy it without really copying it
 * 
 */
class SharedTimer {
    public:
        SharedTimer() {
            timer = new Timer;
            refCounter = new uint32_t;

            (*refCounter) = 1;
        }

        ~SharedTimer() {
            if (--(*refCounter) == 0) {
                delete timer;
                delete refCounter;
            }
        }

        SharedTimer(const SharedTimer& otherTimer) {
            copy(otherTimer);
        }

        SharedTimer& operator=(const SharedTimer& otherTimer) {
            this->~SharedTimer();
            copy(otherTimer);
            return *this;
        }

        void start() {
            timer->start();
        }

        void stop() {
            timer->stop();
        }

        void reset() {
            timer->reset();
        }

        float read() {
            return timer->read();
        }

        unsigned long read_ms() {
            return timer->read_ms();
        }

        operator float() {
            Timer& timerRef = *timer;
            return timerRef;
        }
    
    private:
        Timer* timer;
        uint32_t* refCounter;

        void copy(const SharedTimer& otherTimer) {
            timer = otherTimer.timer;
            refCounter = otherTimer.refCounter;

            ++(*refCounter);
        }
};

#endif // DYNAMIC_TIMER_H