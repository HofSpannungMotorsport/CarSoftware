#ifndef HARDWARE_HV_ENABLED_H
#define HARDWARE_HV_ENABLED_H

#include "../interface/IHvEnabled.h"

enum hv_enabled_on_state_t : bool {
    HV_ENABLED_ON_AT_HIGH = true,
    HV_ENABLED_ON_AT_LOW = false
};

#define STD_HV_DEBOUNCE_TIME 0.1 // s

class HardwareHvEnabled : public IHvEnabled {
    public:
        HardwareHvEnabled(PinName pin, id_sub_component_t componentSubId, hv_enabled_on_state_t onState = HV_ENABLED_ON_AT_HIGH)
        : _pin(pin) {
            setComponentType(COMPONENT_SYSTEM);
            setObjectType(OBJECT_HARDWARE);
            setComponentSubId(componentSubId);

            _debounceTime = STD_HV_DEBOUNCE_TIME;

            if (onState == HV_ENABLED_ON_AT_HIGH) {
                _pin.fall(callback(this, &HardwareHvEnabled::_deactivated));
                _pin.rise(callback(this, &HardwareHvEnabled::_activated));
            } else if (onState == HV_ENABLED_ON_AT_LOW) {
                _pin.fall(callback(this, &HardwareHvEnabled::_activated));
                _pin.rise(callback(this, &HardwareHvEnabled::_deactivated));
            } else {
                #ifdef MESSAGE_REPORT
                    pcSerial.printf("Cannot assign method to a hv-enabled-state. Wrong hv-enabled-type choosen?");
                #endif
            }
        }

        // Get the current Status of HV (if it is enabled or not)
        virtual bool read() {
            #ifdef FORCE_DISABLE_HV_CHECK
                return true;
            #else
                return _currentState;
            #endif
        }

        // Shorthand for read()
        virtual operator bool() {
            return read();
        }
    
    private:
        InterruptIn _pin;
        bool _currentState;
        bool _lastHardwareState, _debouncing, _debounced;
        float _debounceTime;
        Ticker _ticker;

        void _debounce() {
            _ticker.detach();
            _debouncing = false;

            if (_lastHardwareState) {
                _debounced = true;
                _currentState = true;
            }
        }

        void _activated() {
            _lastHardwareState = true;
            if (!_debouncing && !_debounced) {
                _debouncing = true;
                _ticker.attach(callback(this, &HardwareHvEnabled::_debounce), _debounceTime);
            }
        }

        void _deactivated() {
            _lastHardwareState = false;
            if (_debounced)  {
                _ticker.detach();
                _debounced = false;
                _currentState = false;
            }
        }
};

#endif // HARDWARE_HV_ENABLED_H