#ifndef RUNABLESCHEDULER_H
#define RUNABLESCHEDULER_H

#include <vector>
#include "IRunable.h"

using namespace std;

#define STD_SCHEDULER_REFRESH_RATE 5 // Hz

class RunableScheduler : public IRunable {
    public:
        virtual void run() {
            for (auto &service : _services) {
                if (service.running) {
                    if (service.lastRun->read() > (1.0 / service.refreshRate)) {
                        service.lastRun->reset();
                        service.service->run();
                    }
                } else {
                    service.service->run();
                    service.lastRun->reset();
                    service.lastRun->start();
                    service.running = true;
                }
            }
        }

        /*
            Add a Runable to the List.
            Don't use in realtime, its slow caused by 'new', only at configuration/startup
        */
        void addRunable(IRunable* service, float refreshRate = STD_SCHEDULER_REFRESH_RATE) {
            _services.emplace_back(service, refreshRate);
        }

    private:
        class RunableSchedule {
            public:
                RunableSchedule(IRunable* servicePointer, float serviceRefreshRate) {
                    service = servicePointer;
                    refreshRate = serviceRefreshRate;
                    lastRun = new Timer;
                }

                ~RunableSchedule() {
                    delete lastRun;
                }

                RunableSchedule(const RunableSchedule& r) {
                    service = r.service;
                    refreshRate = r.refreshRate;
                    running = false;
                    lastRun = new Timer;
                }

                IRunable *service;
                float refreshRate = STD_SCHEDULER_REFRESH_RATE;
                bool running = false;

                Timer *lastRun;
        };

        vector<RunableSchedule> _services;
};

#endif // RUNABLESCHEDULER_H