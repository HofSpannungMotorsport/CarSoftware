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

        virtual void setStatus(pedal_status_t status) {
            _status = status;
        }
        
        virtual void setValue(pedal_value_t value) {
            _value = value;
        }

    private:
        pedal_status_t _status;
        pedal_value_t _value;

        pedal_calibration_t _calibrationStatus = CURRENTLY_NOT_CALIBRATING;
        bool _calibrationStatusChanged = false;
};

#endif