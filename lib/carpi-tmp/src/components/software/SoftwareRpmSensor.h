#ifndef SOFTWARERPMSENSOR_H
#define SOFTWARERPMSENSOR_H

#include "../interface/IRpmSensor.h"

class SoftwareRpmSensor : public IRpmSensor {
    public:
        SoftwareRpmSensor() {
            setComponentType(COMPONENT_RPM_SENSOR);
            setObjectType(OBJECT_SOFTWARE);
        }

        SoftwareRpmSensor(id_sub_component_t componentSubId)
            : SoftwareRpmSensor() {
            setComponentSubId(componentSubId);
        }

        virtual void setStatus(rpm_sensor_status_t status) {
            _status = status;
        }

        virtual rpm_sensor_status_t getStatus() {
            return _status;
        }

        virtual void setMeasurementPointsPerRevolution(uint8_t measurementPointsPerRevolution) {
            // implementing later...
            // [il]
        }

        virtual void setFrequency(rpm_sensor_frequency_t frequency) {
            _frequency = frequency;
        }

        virtual uint8_t getMeasurementsPerRevolution() {
            // implementing later...
            // [il]
            return 0;
        }

        virtual rpm_sensor_frequency_t getFrequency() {
            return _frequency;
        }

    private:
        rpm_sensor_frequency_t _frequency = 0;
        rpm_sensor_status_t _status = 0;
};

#endif // SOFTWARERPMSENSOR_H