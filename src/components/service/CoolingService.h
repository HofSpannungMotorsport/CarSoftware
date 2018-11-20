#ifndef COOLINGSERVICE_H
#define COOLINGSERVICE_H

#include "mbed.h"
#include "IService.h"
#include "CarService.h"
#include "SpeedService.h"
#include "../interface/IFan.h"
#include "../interface/IPump.h"

#define STD_COOLING_FAN_OFF_UNTIL_SPEED 7 // kM/h
#define STD_COOLING_FAN_ON_UNTIL_SPEED 5 // kM/h

class CoolingService : public IService {
    public:
        CoolingService(CarService &carService,
                       SpeedService &speedService,
                       IFan* fan, IPump* pump,
                       PinName highVoltageOnPin)
            : _carService(carService), _speedService(speedService), _highVoltageOnPin(highVoltageOnPin) {
            _fan = fan;
            _pump = pump;
        }

        virtual void run() {
            if (_highVoltageOnPin) {
                _pump->setSpeed(1);

                speed_value_t currentSpeed = _speedService.getSpeed();

                if (currentSpeed <= STD_COOLING_FAN_ON_UNTIL_SPEED) {
                    _fan->setState(FAN_ON);
                } else if (currentSpeed >= STD_COOLING_FAN_OFF_UNTIL_SPEED) {
                    _fan->setState(FAN_OFF);
                }
            } else {
                _pump->setSpeed(0);
                _fan->setState(FAN_OFF);
            }
        }

    protected:
        CarService &_carService;
        SpeedService &_speedService;
        IFan* _fan;
        IPump* _pump;

        DigitalIn _highVoltageOnPin;
};

#endif // COOLINGSERVICE_H