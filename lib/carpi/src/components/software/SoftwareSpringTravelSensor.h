#ifndef SOFTWARE_SPRING_TRAVEL_SENSOR_H
#define SOFTWARE_SPRING_TRAVEL_SENSOR_H

#include "../interface/ISpringTravelSensor.h"

class SoftwareSpringTravelSensor : public ISpringTravelSensor {
    public:
        SoftwareSpringTravelSensor(id_sub_component_t componentSubId) {
            setComponentType(COMPONENT_SUSPENSION_TRAVEL);
            setObjectType(OBJECT_SOFTWARE);
            setComponentSubId(componentSubId);
        }

        virtual void setProportionality(spring_sensor_type_t proportionality) {
            _sendCommand(SPRING_MESSAGE_COMMAND_SET_PROPORTIONALITY, proportionality, SEND_PRIORITY_SUSPENSION, IS_NOT_DROPABLE);
        }

        virtual status_t getStatus() {
            return _status;
        }

        virtual spring_value_t getValue() {
            return _value;
        }

        virtual void setCalibrationStatus(spring_calibration_t calibrationStatus) {
            _sendCommand(SPRING_MESSAGE_COMMAND_SET_CALIBRATION_STATUS, calibrationStatus, SEND_PRIORITY_SUSPENSION, IS_NOT_DROPABLE);
            _calibrationStatus = calibrationStatus;
        }

        virtual spring_calibration_t getCalibrationStatus() {
            return _calibrationStatus;
        }

        virtual void setStatus(status_t status) {
            _status = status;
        }
        
        virtual void setValue(spring_value_t value) {
            _value = value;
        }

        virtual void setRaw(spring_raw_t raw) {
            _raw = raw;
        }

        virtual void receive(CarMessage &carMessage) {
            for (car_sub_message_t &subMessage : carMessage.subMessages) {
                uint16_t newValue16;
                float newValue;
                switch (subMessage.data[0]) {
                    case SPRING_MESSAGE_COMMAND_SET_STATUS:
                        setStatus(subMessage.data[1]);
                        break;
                    
                    case SPRING_MESSAGE_COMMAND_SET_VALUE:
                        newValue16 = subMessage.data[1] | (subMessage.data[2] << 8);
                        newValue = (float)newValue16 / 65535.0;
                        setValue(newValue);
                        break;
                    
                    case SPRING_MESSAGE_COMMAND_SET_RAW:
                        newValue16 = subMessage.data[1] | (subMessage.data[2] << 8);
                        setRaw(newValue16);
                        break;
                    
                    case SPRING_MESSAGE_COMMAND_SET_BOTH:
                        newValue16 = subMessage.data[1] | (subMessage.data[2] << 8);
                        newValue = (float)newValue16 / 65535.0;
                        setValue(newValue);

                        newValue16 = subMessage.data[3] | (subMessage.data[4] << 8);
                        setRaw(newValue16);
                        break;
                }
            }
        }

    private:
        status_t _status;
        spring_value_t _value;
        spring_raw_t _raw;

        spring_calibration_t _calibrationStatus = SPRING_CURRENTLY_NOT_CALIBRATING;
};

#endif // SOFTWARE_SPRING_TRAVEL_SENSOR_H