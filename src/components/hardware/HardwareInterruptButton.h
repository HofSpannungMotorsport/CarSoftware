#ifndef HARDWARE_INTERRUPT_BUTTON_H
#define HARDWARE_INTERRUPT_BUTTON_H

#include "mbed.h"
#include "../interface/IInterruptButton.h"

#define STD_DEBOUNCE_TIME 0.010 //  s
#define STD_CLICK_TIME       11 // ms
#define STD_LONG_CLICK_TIME 800 // ms

typedef uint8_t button_count_t;

class HardwareInterruptButton : public IInterruptButton {
    public:
        HardwareInterruptButton(PinName pin, button_type_t buttonType = NORMALLY_OPEN) : _interruptPin(pin) {
            _counter.pressed = 0;
            _counter.clicked = 0;
            _counter.longClicked = 0;
            _counter.longClickStarted = 0;

            _time.debounce = STD_DEBOUNCE_TIME;
            _time.click = STD_CLICK_TIME;
            _time.longClick = STD_LONG_CLICK_TIME;

            _lastStatus = false;
            _debouncing = false;
            _debounced = false;

            // Assign the Function/Method to a state (Rising/Falling) after initializing all variables
            if (buttonType == NORMALLY_CLOSED) {
                _interruptPin.fall(callback(this, &HardwareInterruptButton::_pressed));
                _interruptPin.rise(callback(this, &HardwareInterruptButton::_released));
            } else if (buttonType == NORMALLY_OPEN) {
                _interruptPin.rise(callback(this, &HardwareInterruptButton::_pressed));
                _interruptPin.fall(callback(this, &HardwareInterruptButton::_released));
            } else {
                printf("Cannot assign method to a button-state. Wrong button-type choosen?");
            }

        }

        virtual ~HardwareInterruptButton() {

        }

        //Set the Time for the different events
        virtual void setClickTime(button_time_t time) {
            _time.click = time;
        }

        virtual void setLongClickTime(button_time_t time) {
            _time.longClick = time;
        }

        virtual void setDebounceTime(button_debounce_time_t time) {
            _time.debounce = time;
        }

        // Getters
        virtual button_state_t getPress() {
            if (_counter.pressed > 0) {
                _counter.pressed--;
                return STATE_PRESSED;
            } else {
                return STATE_NOT_PRESSED;
            }
        }

        virtual button_state_t getClick() {
            if (_counter.clicked > 0) {
                _counter.clicked--;
                return STATE_PRESSED;
            } else {
                return STATE_NOT_PRESSED;
            }
        }

        virtual button_state_t getLongClick() {
            if (_counter.longClicked > 0) {
                _counter.longClicked--;
                return STATE_PRESSED;
            } else {
                return STATE_NOT_PRESSED;
            }
        }

        virtual button_state_t getLongClickStart() {
            if (_counter.longClickStarted > 0) {
                _counter.longClickStarted--;
                return STATE_PRESSED;
            } else {
                return STATE_NOT_PRESSED;
            }
        }

    private:
        InterruptIn _interruptPin;
        Timer _timerSincePressed;
        Ticker _ticker;
        bool _lastStatus, _debouncing, _debounced;

        struct _counter {
            button_count_t pressed, clicked, longClicked, longClickStarted;
        } _counter;

        struct _time {
            button_time_t click, longClick;
            button_debounce_time_t debounce;
        } _time;

        void _resetTimer(Timer &timer) {
            timer.stop();
            timer.reset();
        }

        void _restartTimer(Timer &timer) {
            _resetTimer(timer);
            timer.start();
        }

        void _debounce() {
            // Called after the set time.debounce. To check if the first rising edge was just a false mesurement
            // or a correct press
            _ticker.detach();
            _debouncing = false;

            if (_lastStatus) {
                _ticker.attach(callback(this, &HardwareInterruptButton::_checkLongClick), (float)(((float)_time.longClick/1000.0) - _time.debounce));
                _debounced = true;
                _counter.pressed++;
            } else {
                _resetTimer(_timerSincePressed);
            }
        }

        void _checkLongClick() {
            _ticker.detach();
            _counter.longClickStarted++;
        }

        void _pressed() {
            _lastStatus = true;
            if (!_debouncing && !_debounced) {
                _debouncing = true;
                _ticker.attach(callback(this, &HardwareInterruptButton::_debounce), _time.debounce);
                _restartTimer(_timerSincePressed);
            }
        }

        void _released() {
            _lastStatus = false;
            if (_debounced)  {
                _ticker.detach();
                button_time_t timeSincePressed = _timerSincePressed.read_ms();
                _resetTimer(_timerSincePressed);

                _debounced = false;
                
                if (timeSincePressed > _time.longClick) {
                    _counter.longClicked++;
                } else if (timeSincePressed > _time.click) {
                    _counter.clicked++;
                }
            }
        }
};

#endif