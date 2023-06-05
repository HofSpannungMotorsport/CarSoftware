#ifndef LEDSERVICE_H
#define LEDSERVICE_H

#include "IService.h"
#include "communication/CANService.h"
#include "components/interface/ILed.h"

#define STARTUP_ANIMATION_SPEED 75000            // s between led-changes
#define STARTUP_ANIMATION_PLAYBACKS 3            // times the animation should be played
#define LED_RED_ON_TIME_PEDAL_COMM_INTERF 500000 // s

class SLed : public IService
{
public:
    SLed(CANService &canService, ILed *ledRed, ILed *ledGreen, ILed *ledBlue, ILed *ledCal, ILed *ledRtd, ILed *ledLct)
        : _canService(canService)
    {

        _led.red = ledRed;
        _led.green = ledGreen;
        _led.blue = ledBlue;
        _led.cal = ledCal;
        _led.rtd = ledRtd;
        _led.lct = ledLct;
    }

    struct _led
    {
        ILed *red;
        ILed *green;
        ILed *blue;
        ILed *cal;
        ILed *rtd;
        ILed *lct;
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
        _led.cal->setBrightness(1);
        _led.rtd->setBrightness(1);
        _led.green->setBrightness(1);
        _led.blue->setBrightness(1);
        _led.lct->setBrightness(1);

        _led.red->setState(LED_OFF);
        _led.cal->setState(LED_OFF);
        _led.rtd->setState(LED_OFF);
        _led.green->setState(LED_OFF);
        _led.blue->setState(LED_OFF);
        _led.lct->setState(LED_OFF);

        _led.green->setBlinking(BLINKING_OFF);
        _led.blue->setBlinking(BLINKING_OFF);
        _led.lct->setBlinking(BLINKING_OFF);
        _led.red->setBlinking(BLINKING_OFF);
        _led.cal->setBlinking(BLINKING_OFF);
        _led.rtd->setBlinking(BLINKING_OFF);
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
        for (uint8_t i = 0; i < STARTUP_ANIMATION_PLAYBACKS; i++)
        {
            resetLeds();
            _led.red->setState(LED_ON);
            _led.green->setState(LED_ON);
            _led.blue->setState(LED_ON);
            run();

            wait_us(STARTUP_ANIMATION_SPEED);

            _led.lct->setState(LED_ON);
            run();

            wait_us(STARTUP_ANIMATION_SPEED);

            _led.cal->setState(LED_ON);
            run();

            wait_us(STARTUP_ANIMATION_SPEED);

            _led.rtd->setState(LED_ON);
            _led.red->setState(LED_OFF);
            run();

            wait_us(STARTUP_ANIMATION_SPEED);

            _led.lct->setState(LED_OFF);
            run();

            wait_us(STARTUP_ANIMATION_SPEED);

            _led.cal->setState(LED_OFF);
            run();

            wait_us(STARTUP_ANIMATION_SPEED);

            _led.rtd->setState(LED_OFF);
            run();

            wait_us(STARTUP_ANIMATION_SPEED);
        }
    }

    /**
     * Indicates diffrent Car States.
     *
     * CAR_OFF:
     *
     * BOOT:
     *
     * ALMOST_READY_TO_DRIVE: Green LED blinking fast
     *
     * READY_TO_DRIVE:
     *
     * CAR_ERROR
     *
     * CALIBRATION_NEEDED:
     *
     * LAUNCH_CONTROL:
     */
    void indicateCarState(car_state_t carState)
    {
        resetLeds();
        printf("Indicating car state: %d\n", carState);
        switch (carState)
        {
        case CAR_OFF:
            /* code */
            _led.lct->setState(LED_ON);
            _led.lct->setBlinking(BLINKING_SLOW);
            break;

        case LV_NOT_CALIBRATED:
            /* code */
            _led.lct->setState(LED_ON);
            _led.lct->setBlinking(BLINKING_SLOW);
            _led.cal->setState(LED_ON);
            _led.cal->setBlinking(BLINKING_SLOW);
            break;
        case HV_CALIBRATING:
        case LV_CALIBRATING:
            _led.cal->setState(LED_ON);
            _led.cal->setBlinking(BLINKING_FAST);

            _led.rtd->setState(LED_ON);
            _led.rtd->setBlinking(BLINKING_SLOW);
            break;
        case LV_CALIBRATED:
            _led.lct->setState(LED_ON);
            _led.lct->setBlinking(BLINKING_SLOW);
            /* code */
            break;

        case HV_NOT_CALIBRATED:
            _led.cal->setState(LED_ON);
            _led.cal->setBlinking(BLINKING_SLOW);
            break;
        case HV_CALIBRATED:
            _led.rtd->setState(LED_ON);
            _led.rtd->setBlinking(BLINKING_SLOW);
            break;
        case DRIVE:
            _led.rtd->setState(LED_ON);

            _led.red->setState(LED_ON);
            _led.green->setState(LED_ON);
            _led.blue->setState(LED_ON);
            break;
        case LAUNCH_CONTROL:
            /* code */
            break;
        case CAR_ERROR:
            _led.red->setState(LED_ON);
            _led.red->setBlinking(BLINKING_NORMAL);
            break;
        default:
            _led.lct->setState(LED_ON);
            _led.cal->setState(LED_ON);
            _led.rtd->setState(LED_ON);
            _led.red->setState(LED_ON);
            _led.green->setState(LED_ON);
            _led.blue->setState(LED_ON);
            break;
        }
        run();
    }

    virtual void run()
    {
        _canService.sendMessage((ICommunication *)_led.red, DEVICE_DASHBOARD);
        _canService.sendMessage((ICommunication *)_led.green, DEVICE_DASHBOARD);
        _canService.sendMessage((ICommunication *)_led.blue, DEVICE_DASHBOARD);
        _canService.sendMessage((ICommunication *)_led.cal, DEVICE_DASHBOARD);
        _canService.sendMessage((ICommunication *)_led.rtd, DEVICE_DASHBOARD);
        _canService.sendMessage((ICommunication *)_led.lct, DEVICE_DASHBOARD);
    }

protected:
    CANService &_canService;
    Ticker _redOffTicker;
};

#endif // LEDSERVICE_H