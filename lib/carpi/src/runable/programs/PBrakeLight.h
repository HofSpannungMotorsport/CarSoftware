#ifndef PBRAKELIGHT_H
#define PBRAKELIGHT_H

#include "IProgram.h"
#include "components/interface/IPedal.h"
#include "components/interface/ILed.h"

class PBrakeLight : public IProgram {
    public:
        PBrakeLight(IPedal &brakePedal, ILed &brakeLight, IRegistry &registry)
            : _brakePedal(brakePedal), _brakeLight(brakeLight), _registry(registry) {
            _brakeLight.setBlinking(BLINKING_OFF);
            _brakeLight.setBrightness(1);
        }

        virtual void run() {
            // If Brakeing -> light on
            // If not Brakeing -> light off
            // If Error or something else -> light on
            pedal_value_t brakePedalValue = _brakePedal.getValue();
            if (_brakePedal.getStatus() == 0) {
                if (brakePedalValue >= _registry.getFloat(BRAKE_LIGHT_UPPER_THRESHHOLD)) {
                    _brakeLight.setState(LED_ON);
                } else if (brakePedalValue < _registry.getFloat(BRAKE_LIGHT_LOWER_THRESHHOLD)) {
                    _brakeLight.setState(LED_OFF);
                }
            } else {
                _brakeLight.setState(LED_OFF);
            }
        }

    protected:
        IRegistry &_registry;

        IPedal &_brakePedal;
        ILed &_brakeLight;
};

#endif // PBRAKELIGHT_H