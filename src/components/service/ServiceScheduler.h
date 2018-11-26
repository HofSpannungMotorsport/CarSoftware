#ifndef SERVICESCHEDULER_H
#define SERVICESCHEDULER_H

#include <vector>
#include <memory>
#include "IService.h"

#define STD_SCHEDULER_REFRESH_RATE 5 // Hz

class ServiceSchedule {
    public:
        ServiceSchedule(IService* servicePointer, float serviceRefreshRate) {
            service = servicePointer;
            refreshRate = serviceRefreshRate;
        }

        IService *service;
        float refreshRate = STD_SCHEDULER_REFRESH_RATE;
        Timer lastRun = Timer(); // Time since service last run
        bool running = false;
};

class ServiceScheduler : public IService {
    public:
        virtual void run() {
            for (auto &service : _services) {
                if (service.running) {
                    if (service.lastRun.read() > (1.0 / service.refreshRate)) {
                        service.lastRun.reset();
                        service.service->run();
                    }
                } else {
                    service.service->run();
                    service.lastRun.reset();
                    service.lastRun.start();
                    service.running = true;
                }
            }
        }

        void addService(IService* service, float refreshRate = STD_SCHEDULER_REFRESH_RATE) {
            addService(ServiceSchedule(service, refreshRate));
        }

        void addService(ServiceSchedule serviceSchedule) {
            _services.emplace_back(serviceSchedule);
        }

    protected:
        vector<ServiceSchedule> _services;
};

#endif // SERVICESCHEDULER_H