#ifndef IANALOG_SENSOR_H
#define IANALOG_SENSOR_H

#include "IComponent.h"

typedef float analog_sensor_t;
typedef uint16_t analog_sensor_raw_t;
typedef float analog_sensor_boundary_t;

enum analog_sensor_error_type_t : status_t {
    DISCONNECTED =                  0x1,
    SHORT_CIRCUIT =                 0x2,
    OUT_OF_BOUNDARY =               0x4,
    BOUNDARY_CHECK_FAIL =           0x8
};

class IAnalogSensor : private NonCopyable<IAnalogSensor> {
    public:
        virtual bool setMapping(analog_sensor_raw_t minIn, analog_sensor_raw_t maxIn, analog_sensor_t minOut, analog_sensor_t maxOut) = 0;
        
        virtual bool setRawBoundary(analog_sensor_raw_t lowerEnd, analog_sensor_raw_t upperEnd) = 0;
        virtual void setRawBoundaryOutTime(uint16_t time) = 0;
        virtual void setBoundary(analog_sensor_boundary_t bounderyPercentage) = 0;
        virtual void setBoundaryOutTime(uint16_t time) = 0;

        virtual analog_sensor_raw_t getRawValue() = 0;
        virtual analog_sensor_t getValue() = 0;
};

#endif