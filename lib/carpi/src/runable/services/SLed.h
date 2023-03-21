#ifndef LEDSERVICE_H
#define LEDSERVICE_H

#include "IService.h"
#include "communication/CANService.h"
#include "components/interface/ILed.h"

class SLed: public IService {
    public:
        SLed(CANService &canService,ILed *ledRed, ILed *ledYellow, ILed *ledGreen)
            : _canService(canService){

            _led.red = ledRed;
            _led.yellow = ledYellow;
            _led.green = ledGreen;
        }

        struct _led
        {
            ILed *red;
            ILed *yellow;
            ILed *green;
        } _led;

        void redOff()
        {
            _redOffTicker.detach();
            _led.red->setState(LED_OFF);
            _canService.sendMessage((ICommunication *)_led.red, DEVICE_DASHBOARD);
        }

        void resetLeds()
        {
            redOff();

            _led.red->setBrightness(1);
            _led.yellow->setBrightness(1);
            _led.green->setBrightness(1);

            _led.red->setState(LED_OFF);
            _led.yellow->setState(LED_OFF);
            _led.green->setState(LED_OFF);

            _led.red->setBlinking(BLINKING_OFF);
            _led.yellow->setBlinking(BLINKING_OFF);
            _led.green->setBlinking(BLINKING_OFF);
        }

        void pedalCommunitactionInterference()
        {
            _redOffTicker.detach();

            _led.red->setBlinking(BLINKING_OFF);
            _led.red->setBrightness(1.0f);
            _led.red->setState(LED_ON);

            _canService.sendMessage((ICommunication *)_led.red, DEVICE_DASHBOARD);
            _redOffTicker.attach(callback(this, &SLed::redOff), (float)LED_RED_ON_TIME_PEDAL_COMM_INTERF);
        }

        void startupAnimation()
        {
            resetLeds();
            _led.red->setState(LED_ON);
            run();

            wait(STARTUP_ANIMATION_SPEED);

            _led.yellow->setState(LED_ON);
            run();

            wait(STARTUP_ANIMATION_SPEED);

            _led.green->setState(LED_ON);
            _led.red->setState(LED_OFF);
            run();

            wait(STARTUP_ANIMATION_SPEED);

            _led.yellow->setState(LED_OFF);
            run();

            wait(STARTUP_ANIMATION_SPEED);

            _led.green->setState(LED_OFF);
            run();

            wait(STARTUP_ANIMATION_SPEED);
        }

        virtual void run() {
            _canService.sendMessage((ICommunication *)_led.red, DEVICE_DASHBOARD);
            _canService.sendMessage((ICommunication *)_led.yellow, DEVICE_DASHBOARD);
            _canService.sendMessage((ICommunication *)_led.green, DEVICE_DASHBOARD);
        }

    protected:
        CANService &_canService;
        Ticker _redOffTicker;

        
        
};

#endif // LEDSERVICE_H