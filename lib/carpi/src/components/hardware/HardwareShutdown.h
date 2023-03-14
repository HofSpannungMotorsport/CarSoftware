#ifndef HARDWARE_SHUTDOWN_H
#define HARDWARE_SHUTDOWN_H

#include "../interface/IShutdown.h"
#include "../../DelayedSwitch.h"

enum shutdown_on_state_t : bool {
    SHUTDOWN_ON_AT_HIGH = true,
    SHUTDOWN_ON_AT_LOW = false
};

#define SHUTDOWN_DISABELING_DEBOUNCE_TIME 100 // ms
#define SHUTDOWN_ENABELING_DEBOUNCE_TIME 100 // ms

class HardwareShutdown : public IShutdown {
    public:
        HardwareShutdown(PinName pin, id_sub_component_t componentSubId, shutdown_on_state_t onState = SHUTDOWN_ON_AT_HIGH)
        : _pin(pin), _switch(false), _onState(onState) {
            setComponentType(COMPONENT_SYSTEM);
            setObjectType(OBJECT_HARDWARE);
            setComponentSubId(componentSubId);

            _switch.setEnableTime(SHUTDOWN_ENABELING_DEBOUNCE_TIME);
            _switch.setDisableTime(SHUTDOWN_DISABELING_DEBOUNCE_TIME);
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
        shutdown_on_state_t _onState;
};

#endif // HARDWARE_SHUTDOWN_H