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

        virtual void receive(CarMessage &carMessage) {
            for (car_sub_message_t &subMessage : carMessage.subMessages) {
                switch (subMessage.data[0]) {
                    case RPM_MESSAGE_COMMAND_SET_STATUS:
                        setStatus(subMessage.data[1]);
                        break;
                    
                    case RPM_MESSAGE_COMMAND_SET_FREQUENCY:
                        uint32_t frequencyBinary = 0;
                        for (uint8_t i = 1; i < 5; i++) {
                            frequencyBinary |= (((uint32_t)subMessage.data[i]) << ((i - 1) * 8));
                        }
                        rpm_sensor_frequency_t frequency = *((rpm_sensor_frequency_t*)&frequencyBinary);
                        setFrequency(frequency);
                        break;
                }
            }
        }

    private:
        rpm_sensor_frequency_t _frequency = 0;
        rpm_sensor_status_t _status = 0;
};

#endif // SOFTWARERPMSENSOR_H