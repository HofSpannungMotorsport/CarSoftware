#ifndef IANALOG_SENSOR_H
#define IANALOG_SENSOR_H

#include <stdint.h>

typedef uint16_t analog_sensor_val_t;
typedef float analog_sensor_normval_t;

enum analog_sensor_alignment_t
{
    SENSOR_ALIGNMENT_PROPORTIONAL, // value increases when meassured size increases
    SENSOR_ALIGNMENT_INPROPORTIONAL // value decreases when meassured size increases
};

class IAnalogSensor
{
    public:
        virtual void setMinValue(analog_sensor_val_t val) = 0;
        virtual void setMaxValue(analog_sensor_val_t val) = 0;
        virtual void setAlignment(analog_sensor_alignment_t alignment) = 0;
        virtual analog_sensor_val_t getRawValue() = 0;
        virtual analog_sensor_normval_t getNormalizedValue() = 0;

};

#endif