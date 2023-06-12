#ifndef DISPLAYSERVICE_H
#define DISPLAYSERVICE_H

#include "IService.h"
#include "SSpeed.h"
#include "../interface/IMotorController.h"
#include "../interface/IDigitalIn.h"

class SDisplay : public IService
{
public:
    SDisplay(CANService &canService, SCar &carService, SSpeed &speedService, IMotorController *motorController, IDisplay *display, IPedal *gasPedal, IPedal *brakePedal, DigitalIn *shutdownAfterAccu, DigitalIn *shutdownAfterBSPD, DigitalIn *shutdownAfterHvd, DigitalIn *shutdownAfterInverter, DigitalIn *shutdownAfterMainhoop, DigitalIn *shutdownAfterTSMS)
        : _canService(canService), _carService(carService), _speedService(speedService)
    {
        _motorController = motorController;
        _gasPedal = gasPedal;
        _brakePedal = brakePedal;
        _display = display;

        _shutdownAfterAccu = shutdownAfterAccu;
        _shutdownAfterBSPD = shutdownAfterBSPD;
        _shutdownAfterHvd = shutdownAfterHvd;
        _shutdownAfterInverter = shutdownAfterInverter;
        _shutdownAfterMainhoop = shutdownAfterMainhoop;
        _shutdownAfterTSMS = shutdownAfterTSMS;

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
            _display->setCurrent(_motorController->getSentCurrent());
            _display->setMotorTemperature(_motorController->getMotorTemp());
            _display->setAirTemperature(_motorController->getAirTemp());
            _display->setBatteryVoltage(_motorController->getDcVoltage());
            _display->setGas((float)_gasPedal->getValue());
            _display->setBrake((float)_brakePedal->getValue());
            _display->setState((uint8_t)_carService.getState());

#ifdef ENABLE_POWER_MENU
            _display->setPowermode(_carService.getCurrentModeId());
#endif

            _display->setShutdownError(0);

            if (_shutdownAfterTSMS->read() == 1)
            {
                _display->setShutdownError(1);
#ifdef DEBUG_SHUTDOWN
                pcSerial.printf("TSMS not closed\n");
#endif
            }
            else if (_shutdownAfterHvd->read() == 1)
            {
                _display->setShutdownError(2);
#ifdef DEBUG_SHUTDOWN
                pcSerial.printf("HVD not closed\n");
#endif
            }
            else if (_shutdownAfterMainhoop->read() == 1)
            {
                _display->setShutdownError(3);
#ifdef DEBUG_SHUTDOWN
                pcSerial.printf("Mainhoop not closed\n");
#endif
            }
            else if (_shutdownAfterBSPD->read() == 1)
            {
                _display->setShutdownError(4);
#ifdef DEBUG_SHUTDOWN
                pcSerial.printf("BSPD not closed\n");
#endif
            }
            else if (_shutdownAfterInverter->read() == 1)
            {
                _display->setShutdownError(5);
#ifdef DEBUG_SHUTDOWN
                pcSerial.printf("Inverter not closed\n");
#endif
            }
            else if (_shutdownAfterAccu->read() == 1)
            {
                _display->setShutdownError(6);
#ifdef DEBUG_SHUTDOWN
                pcSerial.printf("Accu not closed\n");
#endif
            }
            else
            {
#ifdef DEBUG_SHUTDOWN
                pcSerial.printf("No shutdown error\n");
#endif
            }
            _canService.sendMessage((ICommunication *)_display, DEVICE_DISPLAY);

#ifdef SDISPLAY_REPORT_DISPLAY
            pcSerial.printf("[SSpeed]@run: Current Speed: %.3f kM/h\n", _speed);
#endif
        }
    }

protected:
    CANService &_canService;
    SCar &_carService;
    SSpeed &_speedService;
    IMotorController *_motorController;
    IPedal *_gasPedal;
    IPedal *_brakePedal;
    IDisplay *_display;
    Timer tim;
    DigitalIn *_shutdownAfterAccu;
    DigitalIn *_shutdownAfterBSPD;
    DigitalIn *_shutdownAfterHvd;
    DigitalIn *_shutdownAfterInverter;
    DigitalIn *_shutdownAfterMainhoop;
    DigitalIn *_shutdownAfterTSMS;
};

#endif // DISPLAYSERVICE_H