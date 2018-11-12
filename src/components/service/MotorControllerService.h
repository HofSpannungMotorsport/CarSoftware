#ifndef MOTORCONTROLLERSERVICE_H
#define MOTORCONTROLLERSERVICE_H

#include "IService.h"
#include <memory>

class MotorControllerService : public IService {
    public:
        MotorControllerService(shared_ptr<IPedal> gasPedal, shared_ptr<IPedal> brakePedal);

        MotorCOntrollerService(shared_ptr<IPedal> gasPedal, shared_ptr<IPedal> brakePedal,
                               shared_ptr<IRpmSensor> frontLeftWheel,
                               shared_ptr<IRpmSensor> frontRightWheel,
                               shared_ptr<IRpmSensor> rearLeftWheel,
                               shared_ptr<IRpmSensor> rearRightWheel);
};

#endif // MOTORCONTROLLERSERVICE_H