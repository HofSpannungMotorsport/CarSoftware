#ifndef SERVICELIST_H
#define SERVICELIST_H

#include <vector>
#include "IService.h"

class ServiceList : public IService {
    public:
        virtual void run() {
            for (auto service : _services) {
                service->run();
            }
        }

        void addService(IService* service) {
            _services.push_back(service);
        }

    protected:
        vector<IService*> _services;
};

#endif // SERVICELIST_H