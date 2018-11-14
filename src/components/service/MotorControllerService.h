#ifndef MOTORCONTROLLERSERVICE_H
#define MOTORCONTROLLERSERVICE_H

#include "mbed.h"
#include "IService.h"
#include "CarService.h"
#include <memory>

#define STD_MAX_POWER 80 // kW
#define STD_POWER_SET_ON_MOTOR_CONTROLLER 80 // kW

class MotorControllerService : public IService {
    public:
        MotorControllerService(shared_ptr<IPedal> gasPedal, shared_ptr<IPedal> brakePedal);

        MotorCOntrollerService(CarService &carService;
                               IPedal* gasPedal, IPedal* brakePedal,
                               IRpmSensor* frontLeftWheel,
                               IRpmSensor* frontRightWheel,
                               IRpmSensor* rearLeftWheel,
                               IRpmSensor* rearRightWheel);
};

#endif // MOTORCONTROLLERSERVICE_H