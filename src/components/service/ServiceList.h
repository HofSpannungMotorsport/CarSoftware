#ifndef SERVICELIST_H
#define SERVICELIST_H

#include <vector>
#include <memory>
#include "IService.h"

class ServiceList : public IService {
    public:
        virtual void run() {
            for (auto service : _services) {
                service->run();
            }
        }

        void addService(IService* service) {
            _services.emplace_back(service);
        }

    protected:
        vector<IService*> _services;
};

#endif // SERVICELIST_H