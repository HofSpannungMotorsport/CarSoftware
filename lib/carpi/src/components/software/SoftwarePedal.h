#ifndef SOFTWAREPEDAL_H
#define SOFTWAREPEDAL_H

#include "../interface/IPedal.h"

class SoftwarePedal : public IPedal {
    public:
        SoftwarePedal() {
            setComponentType(COMPONENT_PEDAL);
            setObjectType(OBJECT_SOFTWARE);
        }

        SoftwarePedal(id_sub_component_t componentSubId)
            : SoftwarePedal() {
            setComponentSubId(componentSubId);
        }

        virtual void setProportionality(pedal_sensor_type_t proportionality, uint16_t sensorNumber = 0) {
            if (sensorNumber == 0) _sendCommand(PEDAL_MESSAGE_COMMAND_SET_PROPORTIONALITY_SENSOR_1, proportionality, SEND_PRIORITY_PEDAL, STD_PEDAL_MESSAGE_TIMEOUT, IS_NOT_DROPABLE);
            else if (sensorNumber == 1) _sendCommand(PEDAL_MESSAGE_COMMAND_SET_PROPORTIONALITY_SENSOR_2, proportionality, SEND_PRIORITY_PEDAL, STD_PEDAL_MESSAGE_TIMEOUT, IS_NOT_DROPABLE);
        }

        virtual pedal_status_t getStatus() {
            return _status;
        }

        virtual pedal_value_t getValue() {
            return _value;
        }

        virtual void setCalibrationStatus(pedal_calibration_t calibrationStatus) {
            _sendCommand(PEDAL_MESSAGE_COMMAND_SET_CALIBRATION_STATUS, calibrationStatus, SEND_PRIORITY_PEDAL, STD_PEDAL_MESSAGE_TIMEOUT, IS_NOT_DROPABLE);
            _calibrationStatus = calibrationStatus;
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

        virtual void setStatus(pedal_status_t status) {
            _status = status;
        }
        
        virtual void setValue(pedal_value_t value) {
            _value = value;
        }

        virtual void receive(CarMessage &carMessage) {
            for (car_sub_message_t &subMessage : carMessage.subMessages) {
                switch (subMessage.data[0]) {
                    case PEDAL_MESSAGE_COMMAND_SET_STATUS:
                        setStatus(subMessage.data[1]);
                        break;
                    
                    case PEDAL_MESSAGE_COMMAND_SET_VALUE:
                        uint16_t newValue16 = subMessage.data[1] | (subMessage.data[2] << 8);
                        float newValue = (float)newValue16 / 65535.0;
                        setValue(newValue);
                        break;
                }
            }
        }

    private:
        pedal_status_t _status;
        pedal_value_t _value;

        pedal_calibration_t _calibrationStatus = CURRENTLY_NOT_CALIBRATING;
};

#endif