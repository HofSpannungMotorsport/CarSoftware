#ifndef SERVICESCHEDULER_H
#define SERVICESCHEDULER_H

#include <memory>
#include <vector>
#include "IService.h"

#define STD_SCHEDULER_REFRESH_RATE 5 // Hz

class ServiceScheduler : public IService {
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

        void addService(IService* service, float refreshRate = STD_SCHEDULER_REFRESH_RATE) {
            _services.emplace_back();
            ServiceSchedule &serviceReference = _services.back();

            serviceReference.service = service;
            serviceReference.refreshRate = refreshRate;
        }


    protected:
        class ServiceSchedule {
            public:
                ServiceSchedule() {
                    lastRun = std::shared_ptr<Timer>(new Timer());
                }

                ServiceSchedule(IService* servicePointer, float serviceRefreshRate)
                : ServiceSchedule() {
                    service = servicePointer;
                    refreshRate = serviceRefreshRate;
                }

                IService *service;
                float refreshRate = STD_SCHEDULER_REFRESH_RATE;
                bool running = false;

                std::shared_ptr<Timer> lastRun;
        };

        vector<ServiceSchedule> _services;
};

#endif // SERVICESCHEDULER_H