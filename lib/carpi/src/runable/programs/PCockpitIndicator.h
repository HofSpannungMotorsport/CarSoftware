#ifndef P_COCKPIT_INDICATOR_H
#define P_COCKPIT_INDICATOR_H

#include "IProgram.h"
#include "communication/CANService.h"
#include "components/interface/IHvEnabled.h"
#include "components/interface/ILed.h"


class PCockpitIndicator : public IProgram {
    public:
        PCockpitIndicator(CANService &canService, IHvEnabled &hvEnabled, ILed &ciLed)
        : _canService(canService), _hvEnabled(hvEnabled), _ciLed(ciLed) {
            _resendTimer.reset();
            _resendTimer.start();
        }

        virtual void run() {
            if (_hvEnabled.read()) {
                if (greenActive) {
                    _ciLed.setState(LED_OFF);
                    _ciLed.setBlinking(BLINKING_OFF);
                    _ciLed.setBrightness(1.0);
                    greenActive = false;
                }
            } else {
                if (!greenActive) {
                    _ciLed.setState(LED_ON);
                    _ciLed.setBlinking(BLINKING_OFF);
                    _ciLed.setBrightness(1.0);
                    greenActive = true;
                }
            }

            _canService.sendMessage((ICommunication*)&_ciLed, DEVICE_DASHBOARD);
        }

    protected:
        CANService &_canService;
        IHvEnabled &_hvEnabled;
        ILed &_ciLed;
        Timer _resendTimer;

        bool greenActive = false;
};

#endif // P_COCKPIT_INDICATOR_H