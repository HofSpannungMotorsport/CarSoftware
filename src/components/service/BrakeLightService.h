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
            : _carService(carService), _brakePedal(brakePedal), _brakeLight(brakeLight) {}

        virtual void run() {
            if (_carService.getState() == READY_TO_DRIVE) {
                if (_brakePedal->getValue() >= STD_BRAKE_LIGHT_THRESHHOLD) {
                    _brakeLight->setBrightness(1);
                    _brakeLight->setState(LED_ON);

                    if (_brakePedal->getValue() >= STD_HARD_BRAKE_LIGHT_FLASH_THRESHHOLD) {
                        _brakeLight->setBlinking(BLINKING_FAST);
                    } else {
                        _brakeLight->setBlinking(BLINKING_OFF);
                    }
                } else {
                    _brakeLight->setState(LED_OFF);
                }
            } else {
                _brakeLight->setBrightness(1);
                _brakeLight->setState(LED_ON);
                _brakeLight->setBlinking(BLINKING_NORMAL);
            }
        }
    
    protected:
        CarService &_carService;
        IPedal* _brakePedal;
        ILed* _brakeLight;

};

#endif // BRAKELIGHTSERVICE_H