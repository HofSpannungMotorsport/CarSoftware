#ifndef IANALOG_SENSOR_H
#define IANALOG_SENSOR_H

#include <stdint.h>
#include "IID.h"

typedef float analog_sensor_t;
typedef uint16_t analog_sensor_raw_t;
typedef float analog_sensor_boundary_t;

typedef uint8_t analog_sensor_status_t;
enum analog_sensor_error_type_t : uint8_t {
    UNDEFINED_ANALOG_SENSOR_ERROR = 0x1,
    DISCONNECTED =                  0x2,
    SHORT_CIRCUIT =                 0x4,
    OUT_OF_BOUNDARY =               0x8,
    BOUNDARY_CHECK_FAIL =          0x10
};

class IAnalogSensor : public IID {
    public:
        virtual bool setMapping(analog_sensor_raw_t minIn, analog_sensor_raw_t maxIn, analog_sensor_t minOut, analog_sensor_t maxOut);
        
        virtual bool setRawBoundary(analog_sensor_raw_t lowerEnd, analog_sensor_raw_t upperEnd);
        virtual void setRawBoundaryOutTime(uint16_t time);
        virtual void setBoundary(analog_sensor_boundary_t bounderyPercentage);
        virtual void setBoundaryOutTime(uint16_t time);

        virtual analog_sensor_raw_t getRawValue() = 0;
        virtual analog_sensor_t getValue() = 0;
        virtual analog_sensor_status_t getStatus() = 0;
};

#endif