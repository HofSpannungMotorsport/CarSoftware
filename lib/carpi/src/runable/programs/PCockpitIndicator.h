#ifndef P_COCKPIT_INDICATOR_H
#define P_COCKPIT_INDICATOR_H

#include "IProgram.h"
#include "components/interface/IHvEnabled.h"
#include "components/interface/ILed.h"

class PCockpitIndicator : public IProgram {
    public:
        PCockpitIndicator(IHvEnabled &hvEnabled, ILed &ciLed)
        : _hvEnabled(hvEnabled), _ciLed(ciLed) {}

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
        }

    protected:
        IHvEnabled &_hvEnabled;
        ILed &_ciLed;

        bool greenActive = false;
};

#endif // P_COCKPIT_INDICATOR_H