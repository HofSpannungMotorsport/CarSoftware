#ifndef HARDWARE_HV_ENABLED_H
#define HARDWARE_HV_ENABLED_H

#include "../interface/IHvEnabled.h"

class HardwareHvEnabled : public IHvEnabled {
    public:
        HardwareHvEnabled(PinName pin, id_sub_component_t componentSubId)
        : _pin(pin) {
            setComponentType(COMPONENT_SYSTEM);
            setObjectType(OBJECT_HARDWARE);
            setComponentSubId(componentSubId);
        }

        // Get the current Status of HV (if it is enabled or not)
        virtual bool read() {
            #ifdef FORCE_DISABLE_HV_CHECK
                return true;
            #else
                return _pin;
            #endif
        }

        // Shorthand for read()
        virtual operator bool() {
            return read();
        }
    
    private:
        DigitalIn _pin;
};

#endif // HARDWARE_HV_ENABLED_H