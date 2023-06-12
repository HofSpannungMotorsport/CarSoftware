#ifndef SOFTWARE_HV_ENABLED_H
#define SOFTWARE_HV_ENABLED_H

#include "../interface/IHvEnabled.h"
#include "../../DelayedSwitch.h"


#define STD_HV_DISABELING_DEBOUNCE_TIME 100 // ms
#define STD_HV_ENABLEING_DEBOUNCE_TIME 100 // ms

class SoftwareHvEnabled : public IHvEnabled {
    public:
        SoftwareHvEnabled(id_sub_component_t componentSubId, hv_enabled_on_state_t onState = HV_ENABLED_ON_AT_HIGH)
        : _onState(onState) {
            setComponentType(COMPONENT_SYSTEM);
            setObjectType(OBJECT_SOFTWARE);
            setComponentSubId(componentSubId);
        }

        // Get the current Status of HV (if it is enabled or not)
        virtual bool read() {
            #ifdef FORCE_DISABLE_HV_CHECK
                return true;
            #else
                return _onState;
            #endif
        }

        // Shorthand for read()
        virtual operator bool() {
            return read();
        }
    
    private:
        hv_enabled_on_state_t _onState;
};

#endif // SOFTWARE_HV_ENABLED_H