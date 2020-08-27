#ifndef P_COCKPIT_INDICATOR_H
#define P_COCKPIT_INDICATOR_H

#include "IProgram.h"
#include "communication/CANService.h"
#include "components/interface/IHvEnabled.h"
#include "components/interface/ILed.h"

#define CI_RESEND_TIME 3 // s

class PCockpitIndicator : public IProgram {
    public:
        PCockpitIndicator(CANService &canService, IHvEnabled &hvEnabled, ILed &ciLed)
        : _canService(canService), _hvEnabled(hvEnabled), _ciLed(ciLed) {
            _resendTimer.reset();
            _resendTimer.start();
        }

        virtual void run() {
            bool confChanged = false;
            if (_hvEnabled.read()) {
                if (greenActive) {
                    _ciLed.setState(LED_OFF);
                    _ciLed.setBlinking(BLINKING_OFF);
                    _ciLed.setBrightness(1.0);
                    confChanged = true;
                    greenActive = false;
                }
            } else {
                if (!greenActive) {
                    _ciLed.setState(LED_ON);
                    _ciLed.setBlinking(BLINKING_OFF);
                    _ciLed.setBrightness(1.0);
                    confChanged = true;
                    greenActive = true;
                }
            }

            if (confChanged || _resendTimer.read() > CI_RESEND_TIME) {
                _resendTimer.reset();
                _resendTimer.start();
                _canService.sendMessage((ICommunication*)&_ciLed, DEVICE_DASHBOARD);
            }
        }

    protected:
        CANService &_canService;
        IHvEnabled &_hvEnabled;
        ILed &_ciLed;
        Timer _resendTimer;

        bool greenActive = false;
};

#endif // P_COCKPIT_INDICATOR_H