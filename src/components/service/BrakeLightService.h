#ifndef BRAKELIGHTSERVICE_H
#define BRAKELIGHTSERVICE_H

#include "IService.h"
#include "CarService.h"
#include "../interface/IPedal.h"
#include "../interface/ILed.h"

#define STD_BRAKE_LIGHT_THRESHHOLD 0.01 // 1%
#define STD_HARD_BRAKE_LIGHT_FLASH_THRESHHOLD 0.75 // 75%

class BrakeLightService : public IService {
    public:
        BrakeLightService(CarService &carService, IPedal* brakePedal, ILed* brakeLight)
            : _carService(carService), _brakePedal(brakePedal), _brakeLight(brakeLight) {
            _brakeLight->setBlinking(BLINKING_OFF);
            _brakeLight->setBrightness(1);
        }

        virtual void run() {
            // If Brakeing -> light on
            // If not Brakeing -> light off
            // If Error or something else -> light on
            if (_carService.getState() == READY_TO_DRIVE) {
                if (_brakePedal->getValue() >= STD_BRAKE_LIGHT_THRESHHOLD) {
                    _brakeLight->setState(LED_ON);
                } else {
                    _brakeLight->setState(LED_OFF);
                }
            } else {
                _brakeLight->setState(LED_ON);
            }
        }

    protected:
        CarService &_carService;
        IPedal* _brakePedal;
        ILed* _brakeLight;

};

#endif // BRAKELIGHTSERVICE_H