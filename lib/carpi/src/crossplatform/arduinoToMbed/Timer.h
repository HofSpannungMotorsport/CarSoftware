#ifndef TIMER_H
#define TIMER_H

#include "NonCopyable.h"

class Timer : private NonCopyable<Timer> {
    public:
        Timer() {
            reset();
        }

        void start() {
            if (!_running) {
                if (_stopped) {
                    _startedAt = millis() - (_stoppedAt - _startedAt);
                    _stopped = false;
                } else {
                    _startedAt = millis();
                }

                _running = true;
            }
        }

        void stop() {
            if (_running && !_stopped) {
                _stoppedAt = millis();
                _stopped = true;
            }
        }

        void reset() {
            _startedAt = 0;
            _stoppedAt = 0;
            _running = false;
            _stopped = false;
        }

        float read() {
            return (float)read_ms() / 1000.0;
        }

        unsigned long read_ms() {
            unsigned long returnValue;
            
            if(_running) {
                if (_stopped) {
                    returnValue = _stoppedAt - _startedAt;
                } else {
                    returnValue = millis() - _startedAt;
                }
            } else {
                returnValue = 0;
            }

            return returnValue;
        }

        operator float() {
            return read();
        }
    
    private:
        unsigned long _startedAt;
        unsigned long _stoppedAt;
        bool _running;
        bool _stopped;
};

#endif // TIMER_H