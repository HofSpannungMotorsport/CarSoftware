#ifndef DISPLAYSERVICE_H
#define DISPLAYSERVICE_H

#include "IService.h"
#include "SSpeed.h"
#include "../interface/IMotorController.h"
#include "../interface/IDigitalIn.h"

class SDisplay : public IService
{
public:
    SDisplay(CANService &canService, SCar &carService, SSpeed &speedService, IMotorController *motorController, IDisplay *display, IPedal *gasPedal, IPedal *brakePedal, IDigitalIn *bmsOk, IDigitalIn *imdOk, IDigitalIn *a, IDigitalIn *b, IDigitalIn *c, IDigitalIn *d, IDigitalIn *e, IDigitalIn *f)
        : _canService(canService), _carService(carService), _speedService(speedService)
    {
        _motorController = motorController;
        _gasPedal = gasPedal;
        _brakePedal = brakePedal;
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

    virtual void run()
    {
        if (tim.read() > 0.03)
        {
            tim.reset();
            tim.start();

            _display->setSpeed(_speedService.getSpeed());
            _display->setCurrent(_motorController->getCurrent());
            _display->setMotorTemperature(_motorController->getMotorTemp());
            _display->setAirTemperature(_motorController->getAirTemp());
            _display->setBatteryVoltage(_motorController->getDcVoltage());
            _display->setPower(_motorController->getDcVoltage());
            _display->setGas((float)_gasPedal->getValue());
            _display->setBrake((float)_brakePedal->getValue());

#ifdef ENABLE_POWER_MENU
            _display->setPowermode(_carService.getCurrentModeId());
#endif
#ifdef DEBUG_SHUTDOWN
            //_display->setShutdownError(0);
            pcSerial.printf("BSPD: %d \n", _b->read());
            pcSerial.printf("Mainhoop or HVD: %d\n", _d->read());
            pcSerial.printf("Dashboard or Inertia or BOTS: %d\n", _c->read());
            pcSerial.printf("BMS: %d\n", _e->read());
            pcSerial.printf("IMD: %d\n", _f->read());
            pcSerial.printf("\n\n");
#endif
            if (_b->read() == 0)
            {
                _display->setShutdownError(1);
            }
            else if (_d->read() == 0)
            {
                _display->setShutdownError(2);
            }
            else if (_c->read() == 0)
            {
                _display->setShutdownError(3);
            }
            else if (_e->read() == 0)
            {
                _display->setShutdownError(4);
            }
            // else if (_f->read() == 0){
            //_display->setShutdownError(5); }
            else
            {
                _display->setShutdownError(0);
            }
            _canService.sendMessage((ICommunication *)_display, DEVICE_DISPLAY);

#ifdef SDISPLAY_REPORT_DISPLAY
            pcSerial.printf("[SSpeed]@run: Current Speed: %.3f kM/h\n", _speed);
#endif
        }
    }

protected:
    CANService &_canService;
    SSpeed &_speedService;
    SCar &_carService;
    IDigitalIn *_bmsOk;
    IDigitalIn *_imdOk;
    IDigitalIn *_a;
    IDigitalIn *_b; // BSPD
    IDigitalIn *_c;
    IDigitalIn *_d;
    IDigitalIn *_e;
    IDigitalIn *_f; // TS-ON
    IMotorController *_motorController;
    IDisplay *_display;
    Timer tim;
    IPedal *_gasPedal;
    IPedal *_brakePedal;
};

#endif // DISPLAYSERVICE_H