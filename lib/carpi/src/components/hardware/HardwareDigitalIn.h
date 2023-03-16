#ifndef HARDWARE_DIGITALIN_H
#define HARDWARE_DIGITALIN_H

#include "mbed.h"
#include "interface/IDigitalIn.h"


class HardwareDigitalIn :  public IDigitalIn {
    public:
        HardwareDigitalIn(PinName pin): _pin(pin) {
            setComponentType(COMPONENT_SHUTDOWN);
            setObjectType(OBJECT_HARDWARE);
        }

        HardwareDigitalIn(PinName pin, id_sub_component_t componentSubId) : HardwareDigitalIn(pin){
            setComponentSubId(componentSubId);
        }

         HardwareDigitalIn(PinName pin, PinMode mode): _pin(pin, mode) {
            setComponentType(COMPONENT_SHUTDOWN);
            setObjectType(OBJECT_HARDWARE);
        }

        HardwareDigitalIn(PinName pin, PinMode mode, id_sub_component_t componentSubId) : HardwareDigitalIn(pin, mode){
            setComponentSubId(componentSubId);
        }

        virtual int read() {
            return _pin.read();
        }
        virtual void mode(PinMode pull) {
            _pin.mode(pull);
        }

        virtual message_build_result_t buildMessage(CarMessage &carMessage){
            car_sub_message_t subMessage;

            subMessage.length = 1;
            int a = read();
            subMessage.data[0] = a;
            pcSerial.printf("DATA: %d\n",a);

            carMessage.addSubMessage(subMessage);
            return MESSAGE_BUILD_OK;
        }

        virtual message_parse_result_t parseMessage(CarMessage &carMessage)
        {
            message_parse_result_t result = MESSAGE_PARSE_OK;
            for (car_sub_message_t &subMessage : carMessage.subMessages)
            {
                if (subMessage.length != 1) // not a valid message
                    result = MESSAGE_PARSE_ERROR;
            }
            return result;
        }
    private:
        DigitalIn _pin;
};

#endif