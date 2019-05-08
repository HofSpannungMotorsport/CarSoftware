#ifndef IPEDAL_H
#define IPEDAL_H

#include "communication/SelfSyncable.h"

#define STD_PEDAL_VALUE_REFRESH_RATE 60 // Hz
#define STD_PEDAL_MESSAGE_TIMEOUT 0.005 // s

#define STD_PEDAL_STATUS_REFRESH_TIME 0.05 // s -> 50ms

enum pedal_message_command_t : uint8_t {
    PEDAL_MESSAGE_COMMAND_SET_PROPORTIONALITY_SENSOR_1 = 0x0,
    PEDAL_MESSAGE_COMMAND_SET_PROPORTIONALITY_SENSOR_2,
    PEDAL_MESSAGE_COMMAND_SET_STATUS,
    PEDAL_MESSAGE_COMMAND_SET_VALUE,
    PEDAL_MESSAGE_COMMAND_SET_CALIBRATION_STATUS
};

typedef uint8_t pedal_status_t;
enum pedal_error_type_t : uint8_t {
    SENSOR_DEVIANCE_TOO_HIGH =             0x1,
    CALIBRATION_FAILED_TOO_LOW_DEVIANCE =  0x2,
    CALIBRATION_FAILED_TOO_HIGH_DEVIANCE = 0x4,
    CALIBRATION_FAILED_WRONG_CONFIG =      0x8
};

typedef float pedal_value_t;

enum pedal_sensor_type_t : bool {
    DIRECT_PROPORTIONAL =   false,
    INDIRECT_PROPORTIONAL =  true
};

enum pedal_calibration_t : bool {
    CURRENTLY_CALIBRATING = true,
    CURRENTLY_NOT_CALIBRATING = false
};

class IPedal : public SelfSyncable {
    public:
        virtual void setProportionality(pedal_sensor_type_t proportionality, uint16_t sensorNumber = 0) = 0;

        virtual pedal_status_t getStatus() = 0;
        virtual pedal_value_t getValue() = 0;

        virtual void setCalibrationStatus(pedal_calibration_t calibrationStatus) = 0;
        virtual pedal_calibration_t getCalibrationStatus() = 0;

        virtual void setMaxDeviance(pedal_value_t deviance) = 0;
        virtual void setMaxDevianceTime(uint16_t time) = 0;

        virtual void setStatus(pedal_status_t status) = 0;
        virtual void setValue(pedal_value_t value) = 0;
};

#endif