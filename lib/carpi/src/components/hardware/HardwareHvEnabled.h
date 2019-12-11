#ifndef HARDWARE_HV_ENABLED_H
#define HARDWARE_HV_ENABLED_H

#include "../interface/IHvEnabled.h"
#include "HardConfig.h"

enum hv_enabled_on_state_t : bool {
    HV_ENABLED_ON_AT_HIGH = true,
    HV_ENABLED_ON_AT_LOW = false
};

class HardwareHvEnabled : public IHvEnabled {
    public:
        HardwareHvEnabled(PinName pin, id_sub_component_t componentSubId, hv_enabled_on_state_t onState = HV_ENABLED_ON_AT_HIGH)
        : _pin(pin) {
            setObjectType(OBJECT_HARDWARE);
            setComponentSubId(componentSubId);

            if (onState == HV_ENABLED_ON_AT_HIGH) {
                _pin.fall(callback(this, &HardwareHvEnabled::_deactivated));
                _pin.rise(callback(this, &HardwareHvEnabled::_activated));

                if (_pin.read()) {
                    _activated();
                } else {
                    _deactivated();
                }
            } else if (onState == HV_ENABLED_ON_AT_LOW) {
                _pin.fall(callback(this, &HardwareHvEnabled::_activated));
                _pin.rise(callback(this, &HardwareHvEnabled::_deactivated));

                if (_pin.read()) {
                    _deactivated();
                } else {
                    _activated();
                }
            } else {
                #ifdef MESSAGE_REPORT
                    printf("Cannot assign method to a hv-enabled-state. Wrong hv-enabled-type choosen?");
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

        virtual status_t getStatus() {
            return _status;
        }

        virtual void setStatus(status_t status) {
            // No implementation needed
        }
    
    private:
        status_t _status = 0;

        InterruptIn _pin;
        bool _currentState = false;
        float _debounceTime = STD_HV_ENABLED_DEBOUNCE_TIME;
        Ticker _ticker;

        bool _lastHardwareState = false,
             _change = false;


        void _activationChecker() {
            _ticker.detach();

            if (_lastHardwareState) {
                _currentState = true;
                
            }

            _change = false;
        }

        void _deactivationChecker() {
            _ticker.detach();

            if (!_lastHardwareState) {
                _currentState = false;
                
            }

            _change = false;
        }

        void _activated() {
            _lastHardwareState = true;

            if (!_change) {
                _change = true;
                _ticker.attach(callback(this, &HardwareHvEnabled::_activationChecker), _debounceTime);
            }
        }

        void _deactivated() {
            _lastHardwareState = false;

            if (!_change) {
                _change = true;
                _ticker.attach(callback(this, &HardwareHvEnabled::_deactivationChecker), _debounceTime);
            }
        }
};

#endif // HARDWARE_HV_ENABLED_H