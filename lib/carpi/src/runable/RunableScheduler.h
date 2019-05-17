#ifndef RUNABLESCHEDULER_H
#define RUNABLESCHEDULER_H

#include <memory>
#include <vector>
#include "IRunable.h"

using namespace std;

#define STD_SCHEDULER_REFRESH_RATE 5 // Hz

class RunableScheduler : public IRunable {
    public:
        virtual void run() {
            for (auto &runable : _runableSchedules) {
                if (runable.running) {
                    if (runable.lastRun->read() > (1.0 / runable.refreshRate)) {
                        runable.lastRun->reset();
                        runable.runable->run();
                    }
                } else {
                    runable.runable->run();
                    runable.lastRun->reset();
                    runable.lastRun->start();
                    runable.running = true;
                }
            }
        }

        // Add a Runable to the List.
        void addRunable(IRunable* runable, float refreshRate = STD_SCHEDULER_REFRESH_RATE) {
            _runableSchedules.emplace_back(runable, refreshRate);
        }

        // Optimise List for RAM
        void finalize() {
            _runableSchedules.shrink_to_fit();
        }

    private:
        class RunableSchedule {
            public:
                RunableSchedule(IRunable* runablePointer, float runableRefreshRate) {
                    runable = runablePointer;
                    refreshRate = runableRefreshRate;
                    lastRun = std::shared_ptr<Timer>(new Timer());
                }

                IRunable *runable;
                float refreshRate = STD_SCHEDULER_REFRESH_RATE;
                bool running = false;

                std::shared_ptr<Timer> lastRun;
        };

        vector<RunableSchedule> _runableSchedules;
};

#endif // RUNABLESCHEDULER_H