#ifndef SOFTWAREPEDAL_H
#define SOFTWAREPEDAL_H

#include "../interface/IPedal.h"

class SoftwarePedal : public IPedal {
    public:
        SoftwarePedal(id_sub_component_t componentSubId) {
            setObjectType(OBJECT_SOFTWARE);
            setComponentSubId(componentSubId);
        }

        virtual void setProportionality(pedal_sensor_type_t proportionality, uint16_t sensorNumber = 0) {
            if (sensorNumber == 0) _sendCommand(PEDAL_MESSAGE_COMMAND_SET_PROPORTIONALITY_SENSOR_1, proportionality, IS_NOT_DROPABLE);
            else if (sensorNumber == 1) _sendCommand(PEDAL_MESSAGE_COMMAND_SET_PROPORTIONALITY_SENSOR_2, proportionality, IS_NOT_DROPABLE);
        }

        virtual status_t getStatus() {
            return _status;
        }

        virtual pedal_value_t getValue() {
            return _value;
        }

        virtual void setCalibrationStatus(pedal_calibration_t calibrationStatus) {
            _sendCommand(PEDAL_MESSAGE_COMMAND_SET_CALIBRATION_STATUS, calibrationStatus, IS_NOT_DROPABLE);
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

        virtual void setStatus(status_t status) {
            _status = status;
        }
        
        virtual void setValue(pedal_value_t value) {
            _value = value;
        }

        virtual void receive(CarMessage &carMessage) {
            switch (carMessage[0]) {
                case PEDAL_MESSAGE_COMMAND_SET_STATUS:
                    setStatus(carMessage[1]);
                    break;

                case PEDAL_MESSAGE_COMMAND_SET_VALUE:
                    uint16_t newValue16 = carMessage[1] | (carMessage[2] << 8);
                    float newValue = (float)newValue16 / 65535.0;
                    setValue(newValue);
                    break;
            }
        }

    private:
        status_t _status;
        pedal_value_t _value;

        pedal_calibration_t _calibrationStatus = CURRENTLY_NOT_CALIBRATING;
};

#endif