#ifndef BRAKELIGHTSERVICE_H
#define BRAKELIGHTSERVICE_H

#include "IService.h"
#include "CarService.h"
#include "../interface/IPedal.h"
#include "../interface/ILed.h"

#define STD_BRAKE_LIGHT_LOWER_THRESHHOLD 0.20 // 30%
#define STD_BRAKE_LIGHT_UPPER_THRESHHOLD 0.35

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
            pedal_value_t brakePedalValue = _brakePedal->getValue();
            if (_carService.getState() == READY_TO_DRIVE) {
                if (brakePedalValue >= STD_BRAKE_LIGHT_UPPER_THRESHHOLD) {
                    _brakeLight->setState(LED_ON);
                } else if (brakePedalValue < STD_BRAKE_LIGHT_LOWER_THRESHHOLD) {
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