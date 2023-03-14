#ifndef DISPLAYSERVICE_H
#define DISPLAYSERVICE_H

#include "IService.h"
#include "SCar.h"
#include "SSpeed.h"
#include "../interface/IDisplay.h"
#include "../interface/IMotorController.h"
#include "../interface/IShutdown.h"

class SDisplay : public IService {
    public:
        SDisplay(SCar &carService,SSpeed &speedService, IShutdown* imd, IShutdown* bms,
                     IMotorController* motorController,IDisplay* display)
            : _carService(carService), _speedService(speedService) {
            _motorController = motorController;
            _display = display;
            _imd = imd;
            _bms = bms;
        }

        virtual void run() {

        }


    protected:
        SCar &_carService;
        SSpeed &_speedService;
        IMotorController* _motorController;
        IShutdown* _imd;
        IShutdown* _bms;
        IDisplay* _display;

        float _speed;
        float _dcVoltage;

        car_state_t _carState;
};

#endif // SPEEDSERVICE_H