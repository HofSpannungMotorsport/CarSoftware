#ifndef HARDWARE_HV_ENABLED_H
#define HARDWARE_HV_ENABLED_H

#include "../interface/IHvEnabled.h"
#include "../../DelayedSwitch.h"

enum hv_enabled_on_state_t : bool {
    HV_ENABLED_ON_AT_HIGH = true,
    HV_ENABLED_ON_AT_LOW = false
};

#define STD_HV_DISABELING_DEBOUNCE_TIME 100 // ms
#define STD_HV_ENABLEING_DEBOUNCE_TIME 100 // ms

class HardwareHvEnabled : public IHvEnabled {
    public:
        HardwareHvEnabled(PinName pin, id_sub_component_t componentSubId, hv_enabled_on_state_t onState = HV_ENABLED_ON_AT_HIGH)
        : _pin(pin), _switch(false), _onState(onState) {
            setComponentType(COMPONENT_SYSTEM);
            setObjectType(OBJECT_HARDWARE);
            setComponentSubId(componentSubId);

            _switch.setEnableTime(STD_HV_ENABLEING_DEBOUNCE_TIME);
            _switch.setDisableTime(STD_HV_DISABELING_DEBOUNCE_TIME);
        }

        // Get the current Status of HV (if it is enabled or not)
        virtual bool read() {
            #ifdef FORCE_DISABLE_HV_CHECK
                return true;
            #else
                if (!_onState) {
                    return _switch.set(!_pin.read());
                } // -> else

                return _switch.set(_pin.read());
            #endif
        }

        // Shorthand for read()
        virtual operator bool() {
            return read();
        }
    
    private:
        DigitalIn _pin;
        DelayedSwitch _switch;
        hv_enabled_on_state_t _onState;
};

#endif // HARDWARE_HV_ENABLED_H