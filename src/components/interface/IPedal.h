#ifndef IPEDAL_H
#define IPEDAL_H

#include <stdint.h>
#include "IID.h"

typedef uint8_t pedal_status_t;
enum pedal_error_type_t : uint8_t {
    UNDEFINED_PEDAL_ERROR =               0x1,
    ANALOG_SENSOR_ERROR =                 0x2,
    SENSOR_DEVIANCE_TOO_HIGH =            0x4,
    CALIBRATION_FAILED_TOO_LOW_DEVIANCE = 0x8,
    CALIBRATION_FAILED_WRONG_CONFIG =    0x10
};

typedef float pedal_value_t;

enum pedal_sensor_type_t : bool {
    DIRECT_PROPORTIONAL =   false,
    INDIRECT_PROPORTIONAL =  true
};

class IPedal : public IID {
    public:
        virtual void setProportionality(pedal_sensor_type_t proportionality, uint16_t sensorNumber = 0) = 0;

        virtual pedal_status_t getStatus() = 0;
        virtual pedal_value_t getValue() = 0;

        virtual void beginCalibration() = 0;
        virtual void endCalibration() = 0;

        virtual void setMaxDeviance(pedal_value_t deviance);
        virtual void setMaxDevianceTime(uint16_t time);

        virtual void setStatus(pedal_status_t status) = 0;
        virtual void setValue(pedal_value_t value) = 0;
};

#endif