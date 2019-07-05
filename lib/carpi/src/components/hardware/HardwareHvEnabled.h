#ifndef HARDWARE_HV_ENABLED_H
#define HARDWARE_HV_ENABLED_H

#include "../interface/IHvEnabled.h"

enum hv_enabled_on_state_t : bool {
    HV_ENABLED_ON_AT_HIGH = true,
    HV_ENABLED_ON_AT_LOW = false
};

class HardwareHvEnabled : public IHvEnabled {
    public:
        HardwareHvEnabled(PinName pin, id_sub_component_t componentSubId, hv_enabled_on_state_t onState = HV_ENABLED_ON_AT_HIGH)
        : _pin(pin), _onState(onState) {
            setComponentType(COMPONENT_SYSTEM);
            setObjectType(OBJECT_HARDWARE);
            setComponentSubId(componentSubId);
        }

        // Get the current Status of HV (if it is enabled or not)
        virtual bool read() {
            #ifdef FORCE_DISABLE_HV_CHECK
                return true;
            #else
                if (_onState == HV_ENABLED_ON_AT_HIGH) {
                    return _pin;
                } else if (_onState == HV_ENABLED_ON_AT_LOW) {
                    return !(_pin);
                } else {
                    return false;
                }
            #endif
        }

        // Shorthand for read()
        virtual operator bool() {
            return read();
        }
    
    private:
        DigitalIn _pin;
        hv_enabled_on_state_t _onState;
};

#endif // HARDWARE_HV_ENABLED_H