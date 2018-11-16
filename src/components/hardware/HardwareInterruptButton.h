#ifndef HARDWAREINTERRUPTBUTTON_H
#define HARDWAREINTERRUPTBUTTON_H

#include "mbed.h"
#include "../interface/IButton.h"
#include "platform/CircularBuffer.h"

typedef uint8_t button_count_t;

enum button_event_t {
    EVENT_HAPPENED = true,
    EVENT_NOT_HAPPENED = false
};

class HardwareInterruptButton : public IButton {
    public:
        HardwareInterruptButton(PinName pin, button_type_t buttonType = NORMALLY_OPEN) : _interruptPin(pin) {
            _time.debounce = STD_DEBOUNCE_TIME;
            _time.longClick = STD_LONG_CLICK_TIME;

            _lastHardwareState = false;
            _debouncing = false;
            _debounced = false;

            _status = 0;
            _lastState = NOT_PRESSED;

            _telegramTypeId = BUTTON;
            _objectType = HARDWARE_OBJECT;

            // Assign the Function/Method to a state (Rising/Falling) after initializing all variables
            if (buttonType == NORMALLY_CLOSED) {
                _interruptPin.fall(callback(this, &HardwareInterruptButton::_pressed));
                _interruptPin.rise(callback(this, &HardwareInterruptButton::_released));
            } else if (buttonType == NORMALLY_OPEN) {
                _interruptPin.fall(callback(this, &HardwareInterruptButton::_released));
                _interruptPin.rise(callback(this, &HardwareInterruptButton::_pressed));
            } else {
                #ifdef MESSAGE_REPORT
                    pcSerial.printf("Cannot assign method to a button-state. Wrong button-type choosen?");
                #endif
            }
        }

        HardwareInterruptButton(PinName pin, can_component_t componentId, button_type_t buttonType = NORMALLY_OPEN)
            : HardwareInterruptButton(pin, buttonType) {
            _componentId = componentId;
        }

        virtual void setLongClickTime(button_time_t time) {
            _time.longClick = time;
        }

        virtual void setDebounceTime(button_debounce_time_t time) {
            _time.debounce = time;
        }

        // Status...
        virtual void setStatus(button_status_t status) {
            // No implementation needed
        }

        virtual button_status_t getStatus() {
            return _status;
        }

        // State...
        virtual void setState(button_state_t status) {
            // No Implementation needed
        }

        virtual button_state_t getState() {
            if (getStateChanged()) {
                _lastState = _readState();
            }

            return _lastState;
        }

        virtual bool getStateChanged() {
            if (_stateBuffer.empty())
                return false;
            else
                return true;
        }

    protected:
        InterruptIn _interruptPin;
        Ticker _ticker;
        bool _lastHardwareState, _debouncing, _debounced;
        button_status_t _status;
        button_state_t _lastState;
        CircularBuffer<button_state_t, STATE_BUFFER_SIZE> _stateBuffer;

        struct _time {
            button_time_t longClick;
            button_debounce_time_t debounce;
        } _time;

        // Called after a rising edge and the set time.debounce. To check if the first rising edge
        // was just a false mesurement or a correct press
        void _debounce() {
            _ticker.detach();
            _debouncing = false;

            if (_lastHardwareState) {
                _ticker.attach(callback(this, &HardwareInterruptButton::_checkLongClick), (float)(((float)_time.longClick/1000.0) - _time.debounce));
                _debounced = true;
                _addState(PRESSED);
            }
        }

        // Called if a rising edge occoured and the button was correctly debounced and after
        // the set time.longClick to check if a longClick has started.
        void _checkLongClick() {
            _ticker.detach();
            _addState(LONG_CLICKED);
        }

        // Called every time a rising edge occurs
        void _pressed() {
            _lastHardwareState = true;
            if (!_debouncing && !_debounced) {
                _debouncing = true;
                _ticker.attach(callback(this, &HardwareInterruptButton::_debounce), _time.debounce);
            }
        }

        // Called every time a falling edge occurs
        void _released() {
            _lastHardwareState = false;
            if (_debounced)  {
                _ticker.detach();

                _debounced = false;
                
                _addState(NOT_PRESSED);
            }
        }

        void _addState(button_state_t state) {
            if (_stateBuffer.full())
                _stateBufferFull();

            _stateBuffer.push(state);
        }

        button_state_t _readState() {
            if (_stateBuffer.empty())
                return _lastState;

            button_state_t state = WRONG;
            _stateBuffer.pop(state);

            if (state == WRONG) {
                _status |= WRONG_STATE;
            }

            return state;
        }

        void _stateBufferFull() {
            _status |= STATE_BUFFER_FULL;
        }
};

#endif // HARDWAREINTERRUPTBUTTON_H