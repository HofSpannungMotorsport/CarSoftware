#ifndef DISPLAYSERVICE_H
#define DISPLAYSERVICE_H

#include "IService.h"
#include "SSpeed.h"
#include "../interface/IMotorController.h"
#include "../interface/IDigitalIn.h"

class SDisplay: public IService {
    public:
        SDisplay(CANService &canService, SCar &carService, SSpeed &speedService, IMotorController* motorController, IDisplay* display, IDigitalIn* bmsOk, IDigitalIn* imdOk, IDigitalIn* a , IDigitalIn* b , IDigitalIn* c , IDigitalIn* d , IDigitalIn* e , IDigitalIn* f)
            : _canService(canService), _carService(carService), _speedService(speedService){
            _motorController = motorController;
            _display = display;
            _bmsOk = bmsOk;
            _imdOk = imdOk;
            _a = a;
            _b = b;
            _c = c;
            _d = d;
            _e = e;
            _f = f;
            tim.reset();
            tim.start();
        }

        virtual void run() {
            if(tim.read()> 0.03){
            tim.reset();
            tim.start();
            }

            _display->setSpeed(_speedService.getSpeed());
            _display->setCurrent(_motorController->getCurrent());
            _display->setMotorTemperature(_motorController->getMotorTemp());
            _display->setAirTemperature(_motorController->getAirTemp());
            _display->setBatteryVoltage(_motorController->getDcVoltage());
            _display->setPower(_motorController->getDcVoltage());
        #ifdef ENABLE_POWER_MENU
            _display->setPowermode(_carService.getCurrentModeId());
        #endif

            
            if(_f->read() == 0)
                _display->setShutdownError(1);
            else if(_e->read() == 0)
                _display->setShutdownError(2);
            else if(_d->read() == 0)
                _display->setShutdownError(3);
            else if(_c->read() == 0)
                _display->setShutdownError(4);
            else if(_b->read() == 0)
                _display->setShutdownError(5);
            else if(_a->read() == 0)
                _display->setShutdownError(6);
            else
                _display->setShutdownError(0);

            _canService.sendMessage((ICommunication*)_display, DEVICE_DISPLAY);
            
            #ifdef SDISPLAY_REPORT_DISPLAY
                pcSerial.printf("[SSpeed]@run: Current Speed: %.3f kM/h\n", _speed);
            #endif
        }

    protected:
        CANService &_canService;
        SSpeed &_speedService;
        SCar &_carService;
        IDigitalIn* _bmsOk;
        IDigitalIn* _imdOk;
        IDigitalIn* _a;
        IDigitalIn* _b;
        IDigitalIn* _c;
        IDigitalIn* _d;
        IDigitalIn* _e;
        IDigitalIn* _f;
        IMotorController* _motorController;
        IDisplay* _display;
        Timer tim;
};

#endif // DISPLAYSERVICE_H