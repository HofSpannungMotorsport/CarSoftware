#ifndef DISPLAYSERVICE_H
#define DISPLAYSERVICE_H

#include "IService.h"
#include "SSpeed.h"
#include "../interface/IMotorController.h"
#include "../interface/IDigitalIn.h"

class SDisplay: public IService {
    public:
        SDisplay(CANService &canService, SSpeed &speedService, IMotorController* motorController, IDigitalIn* bmsOk, IDigitalIn* imdOk)
            : _canService(canService), _speedService(speedService), _bmsOk(bmsOk), _imdOk(imdOk) {
            _motorController = motorController;
            _bmsOk = bmsOk;
            _imdOk = imdOk;
        }

        virtual void run() {
            _canService.sendMessage((ICommunication*)_motorController, DEVICE_DISPLAY);
            _canService.sendMessage((ICommunication*)_bmsOk, DEVICE_DISPLAY);
            _canService.sendMessage((ICommunication*)_imdOk, DEVICE_DISPLAY);
            
            #ifdef SDISPLAY_REPORT_DISPLAY
                pcSerial.printf("[SSpeed]@run: Current Speed: %.3f kM/h\n", _speed);
            #endif
        }

    protected:
        CANService &_canService;
        SSpeed &_speedService;
        IDigitalIn* _bmsOk;
        IDigitalIn* _imdOk;
        IMotorController* _motorController;
};

#endif // DISPLAYSERVICE_H