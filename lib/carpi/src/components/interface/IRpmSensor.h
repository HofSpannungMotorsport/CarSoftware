#ifndef IRPMSENSOR_H
#define IRPMSENSOR_H

#include "communication/SelfSyncable.h"

#define RPM_SENSOR_MESSAGE_REFRESH_RATE 120 // Hz

#define RPM_SD_LOG_COUNT 1
enum sd_log_id_rpm_t : sd_log_id_t {
    SD_LOG_ID_RPM = 1
};

enum rpm_message_command_t : uint8_t {
    RPM_MESSAGE_COMMAND_SET_STATUS = 0x0,
    RPM_MESSAGE_COMMAND_SET_FREQUENCY
};

enum rpm_sensor_error_type_t : status_t {
    
};

typedef float rpm_sensor_frequency_t; // rpm

class IRpmSensor : public SelfSyncable {
    public:
        IRpmSensor() {
            setComponentType(COMPONENT_RPM_SENSOR);
        }

        virtual void setMeasurementPointsPerRevolution(uint8_t measurementPointsPerRevolution) = 0;
        virtual void setFrequency(rpm_sensor_frequency_t frequency) = 0;

        virtual uint8_t getMeasurementsPerRevolution() = 0;
        virtual rpm_sensor_frequency_t getFrequency() = 0;

        virtual sd_log_id_t getLogValueCount() {
            return RPM_SD_LOG_COUNT;
        }

        virtual void getLogValue(string &logValue, sd_log_id_t logId) {
            if (logId != 0) return;

            char buffer[12];
            sprintf(buffer, "%.5f", getFrequency());
            logValue = buffer;
        }
};

#endif // IRPMSENSOR_H