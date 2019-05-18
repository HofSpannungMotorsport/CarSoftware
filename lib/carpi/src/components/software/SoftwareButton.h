#ifndef SOFTWAREBUTTON_H
#define SOFTWAREBUTTON_H

#include "../interface/IButton.h"


class SoftwareButton : public IButton {
    public:
        SoftwareButton() {
            _status = 0;
            _lastState = NOT_PRESSED;
            
            setComponentType(COMPONENT_BUTTON);
            setObjectType(OBJECT_SOFTWARE);
        }

        SoftwareButton(id_sub_component_t componentSubId)
            : SoftwareButton() {
            setComponentSubId(componentSubId);
        }

        virtual void setLongClickTime(button_time_t time) {
            // No Implementation needed
        }

        virtual void setDebounceTime(button_debounce_time_t time) {
            // No Implementation needed
        }

        // Status...
        virtual void setStatus(status_t status) {
            _status = status;
        }

        virtual status_t getStatus() {
            return _status;
        }

        // State...
        virtual void setState(button_state_t state) {
            CircularBuffer<button_state_t, STATE_BUFFER_SIZE> stateBufferCopy = _stateBuffer;

            // Get last State saved in Buffer
            button_state_t lastState = _lastState;
            while(!stateBufferCopy.empty()) {
                stateBufferCopy.pop(lastState);
            }

            if (lastState == state) {
                // The new state to add is the same as the last saved state
                // -> Physically unpossible
                _status |= ADDED_SAME_STATE_AS_LAST_TIME;
            } else {
                // The new state to add is different to the last one in the buffer
                // -> Add it
                _addState(state);
            }
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

        virtual void receive(CarMessage &carMessage) {
            for(car_sub_message_t &subMessage : carMessage.subMessages) {
                switch(subMessage.data[0]) {
                    case BUTTON_MESSAGE_COMMAND_ADD_STATE:
                        setState((button_state_t)subMessage.data[1]);
                        break;
                }
            }
        }

    private:
        status_t _status;
        button_state_t _lastState;
        CircularBuffer<button_state_t, STATE_BUFFER_SIZE> _stateBuffer;

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

#endif // SOFTWAREBUTTON_H