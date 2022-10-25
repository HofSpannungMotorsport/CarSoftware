#ifndef SOFTWAREBATTERY_H
#define SOFTWAREBATTERY_H

#include "../interface/IBattery.h"

class SoftwareBattery : public IBattery {
    public:
        SoftwareBattery() {
            setComponentType(COMPONENT_DISPLAY);
            setObjectType(OBJECT_SOFTWARE);
           // _valueAge.start();
        }

        SoftwareBattery(id_sub_component_t componentSubId)
            : SoftwareBattery() {
            setComponentSubId(componentSubId);
        }

        virtual battery_status_t getStatus() {
            return _status;
        }

        virtual battery_voltage_t getVoltage() {
            return _value;
        }

        virtual message_build_result_t buildMessage(CarMessage &carMessage) {
            car_sub_message_t subMessage;

            subMessage.length = 1;

            #ifdef BATTERY_MESSAGE_HANDLER_DEBUG
                pcSerial.printf("[SoftwarePedal]@buildMessage: SoftwareObject calibrationStatus: 0x%x\tmsg.data[0]: 0x%x\tvalueToSent: %i\n", calibrationStatus, subMessage.data[0], valueToSent);
            #endif

            carMessage.addSubMessage(subMessage);

            return MESSAGE_BUILD_OK;
        }

        virtual message_parse_result_t parseMessage(CarMessage &carMessage) {
            message_parse_result_t result = MESSAGE_PARSE_OK;
            for (car_sub_message_t &subMessage : carMessage.subMessages) {
                if(subMessage.length != 3) // not a valid message
                    result = MESSAGE_PARSE_ERROR;

                this->setStatus(subMessage.data[0]);

                // change line below when type of battery_voltage_t changes
                uint16_t newValue16 = subMessage.data[1] | (subMessage.data[2] << 8);
                float newValue = (float)newValue16 / 65535.0;

                this->setValue(newValue);
                // _valueAge.reset();
                //_valueAge.start();

                #ifdef PEDAL_MESSAGE_HANDLER_DEBUG
                    pcSerial.printf("[SoftwarePedal]@parseMessage: HardwareObject (float)batteryValue: %.3f\t(uint16_t)batteryValue: %i\tmsg.data[1]: 0x%x\tmsg.data[2]: 0x%x\n", newValue, newValue16, subMessage.data[1], subMessage.data[2]);
                #endif

            }
            
            return result;
        }
        /*

        float getValueAge() {
            return _valueAge.read();
        }

        void resetAge() {
            _valueAge.reset();
            _valueAge.start();
        }
*/

    private:
        battery_status_t _status;
        battery_voltage_t _value;
        //Timer _valueAge;

        virtual void setStatus(battery_status_t status) {
            _status = status;
        }
        
        virtual void setValue(battery_voltage_t value) {
            _value = value;
        }
};

#endif