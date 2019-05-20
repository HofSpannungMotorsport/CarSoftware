#ifndef ISPRING_TRAVEL_SENSOR_H
#define ISPRING_TRAVEL_SENSOR_H

#include "communication/SelfSyncable.h"
#include "../interface/IAnalogSensor.h"

#define STD_SPRING_VALUE_REFRESH_RATE 60 // Hz

#define STD_SPRING_STATUS_REFRESH_TIME 0.05 // s -> 50ms

#define SPRING_SD_LOG_COUNT 2
enum sd_log_id_spring_t : sd_log_id_t {
    SD_LOG_ID_SPRING_POSITION_RAW = 0,
    SD_LOG_ID_SPRING_POSITION = 1
};

enum spring_message_command_t : uint8_t {
    SPRING_MESSAGE_COMMAND_SET_PROPORTIONALITY = 0x0,
    SPRING_MESSAGE_COMMAND_SET_STATUS,
    SPRING_MESSAGE_COMMAND_SET_VALUE,
    SPRING_MESSAGE_COMMAND_SET_RAW,
    SPRING_MESSAGE_COMMAND_SET_BOTH,
    SPRING_MESSAGE_COMMAND_SET_CALIBRATION_STATUS
};

enum spring_error_type_t : status_t {
    SPRING_CALIBRATION_FAILED_TOO_LOW_DEVIANCE =  0x1,
    SPRING_CALIBRATION_FAILED_TOO_HIGH_DEVIANCE = 0x2,
    SPRING_CALIBRATION_FAILED_WRONG_CONFIG =      0x4
};

typedef analog_sensor_t spring_value_t;
typedef analog_sensor_raw_t spring_raw_t;

enum spring_sensor_type_t : bool {
    SPRING_DIRECT_PROPORTIONAL =   false,
    SPRING_INDIRECT_PROPORTIONAL =  true
};

enum spring_calibration_t : bool {
    SPRING_CURRENTLY_CALIBRATING = true,
    SPRING_CURRENTLY_NOT_CALIBRATING = false
};

class ISpringTravelSensor : public SelfSyncable {
    public:
        virtual void setProportionality(spring_sensor_type_t proportionality) = 0;

        virtual spring_value_t getValue() = 0;
        virtual spring_raw_t getRaw() = 0;

        virtual void setCalibrationStatus(spring_calibration_t calibrationStatus) = 0;
        virtual spring_calibration_t getCalibrationStatus() = 0;

        virtual void setValue(spring_value_t value) = 0;
        virtual void setRaw(spring_raw_t raw) = 0;

        virtual sd_log_id_t getLogValueCount() {
            return SPRING_SD_LOG_COUNT;
        }

        virtual void getLogValue(string &logValue, sd_log_id_t logId) {
            if (logId > SPRING_SD_LOG_COUNT) return;

            char buffer[7];

            switch(logId) {
                case SD_LOG_ID_SPRING_POSITION_RAW:
                    sprintf(buffer, "%i", getRaw());
                    break;
                
                case SD_LOG_ID_SPRING_POSITION:
                    sprintf(buffer, "%1.5f", getValue());
                    break;
            }
            
            logValue = buffer;
        }
};

#endif // ISPRING_TRAVEL_SENSOR_H