#ifndef SOFTWAREPEDAL_H
#define SOFTWAREPEDAL_H

#include "../interface/IPedal.h"

class SoftwarePedal : public IPedal {
    public:
        SoftwarePedal() {
            setComponentType(COMPONENT_PEDAL);
            setObjectType(OBJECT_SOFTWARE);
            _valueAge.start();
        }

        SoftwarePedal(id_sub_component_t componentSubId)
            : SoftwarePedal() {
            setComponentSubId(componentSubId);
        }

        virtual void setProportionality(pedal_sensor_type_t proportionality, uint16_t sensorNumber = 0) {
            // implementing later...
            // [il]
        }

        virtual pedal_status_t getStatus() {
            return _status;
        }

        virtual pedal_value_t getValue() {
            return _value;
        }

        virtual void setCalibrationStatus(pedal_calibration_t calibrationStatus) {
            if (calibrationStatus != _calibrationStatus) {
                _calibrationStatus = calibrationStatus;
                _calibrationStatusChanged = true;
            }
        }

        virtual pedal_calibration_t getCalibrationStatus() {
            return _calibrationStatus;
        }

        virtual void setMaxDeviance(pedal_value_t deviance) {
            // implementing later...
            // [il]
        }

        virtual void setMaxDevianceTime(uint16_t time) {
            // implementing later...
            // [il]
        }

        virtual message_build_result_t buildMessage(CarMessage &carMessage) {
            car_sub_message_t subMessage;

            subMessage.length = 1;

            pedal_calibration_t calibrationStatus = this->getCalibrationStatus();
            uint8_t valueToSent = 0;

            if (calibrationStatus == CURRENTLY_NOT_CALIBRATING) {
                valueToSent = 1;
            } else if (calibrationStatus == CURRENTLY_CALIBRATING) {
                valueToSent = 2;
            }

            subMessage.data[0] = valueToSent;

            #ifdef PEDAL_MESSAGE_HANDLER_DEBUG
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

                // change line below when type of pedal_value_t changes
                uint16_t newValue16 = subMessage.data[1] | (subMessage.data[2] << 8);
                float newValue = (float)newValue16 / 65535.0;

                this->setValue(newValue);
                _valueAge.reset();
                _valueAge.start();

                #ifdef PEDAL_MESSAGE_HANDLER_DEBUG
                    pcSerial.printf("[SoftwarePedal]@parseMessage: HardwareObject (float)pedalValue: %.3f\t(uint16_t)pedalValue: %i\tmsg.data[1]: 0x%x\tmsg.data[2]: 0x%x\n", newValue, newValue16, subMessage.data[1], subMessage.data[2]);
                #endif

            }
            
            return result;
        }

        float getValueAge() {
            return _valueAge.read();
        }

    private:
        pedal_status_t _status;
        pedal_value_t _value;
        Timer _valueAge;

        pedal_calibration_t _calibrationStatus = CURRENTLY_NOT_CALIBRATING;
        bool _calibrationStatusChanged = false;
        

        virtual void setStatus(pedal_status_t status) {
            _status = status;
        }
        
        virtual void setValue(pedal_value_t value) {
            _value = value;
        }
};

#endif