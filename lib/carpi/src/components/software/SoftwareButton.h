#ifndef SOFTWAREBUTTON_H
#define SOFTWAREBUTTON_H

#include "platform/CircularBuffer.h"
#include "../interface/IButton.h"


class SoftwareButton : public IButton {
    public:
        SoftwareButton() {
            _status = 0;
            _lastState = NOT_PRESSED;
            
            setComponentType(COMPONENT_BUTTON);
            setObjectType(OBJECT_SOFTWARE);

            _stateAge.reset();
            _stateAge.start();
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
        virtual void setStatus(button_status_t status) {
            _status = status;
        }

        virtual button_status_t getStatus() {
            return _status;
        }

        // State...
        virtual void setState(button_state_t state) {
            _lastState = state;
        }

        virtual button_state_t getState() {
            return _lastState;
        }

        float getStateAge() {
            return _stateAge.read();
        }

        virtual message_build_result_t buildMessage(CarMessage &carMessage) {
            // No implementation needed yet
            return MESSAGE_BUILD_ERROR;
        }

        virtual message_parse_result_t parseMessage(CarMessage &carMessage) {
            message_parse_result_t result = MESSAGE_PARSE_OK;
            for(car_sub_message_t &subMessage : carMessage.subMessages) {
                if (subMessage.length != 2) result = MESSAGE_PARSE_ERROR; 
                this->setState((button_state_t)subMessage.data[0]);
                this->setStatus((button_status_t)subMessage.data[1]);
            }

            _stateAge.reset();
            _stateAge.start();

            return result;
        }

    private:
        button_status_t _status;
        button_state_t _lastState;
        Timer _stateAge;
};

#endif // SOFTWAREBUTTON_H