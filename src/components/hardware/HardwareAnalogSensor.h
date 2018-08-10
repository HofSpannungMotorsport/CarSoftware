#ifndef HARDWARE_ANALOGSENSOR_H
#define HARDWARE_ANALOGSENSOR_H

#include "IAnalogSensor.h"
#include "mbed.h"

class HardwareAnalogSensor : virtual public IAnalogSensor
{
    private:
        AnalogIn *_input;
        analog_sensor_alignment_t _alignment;
        analog_sensor_normval_t _minValue, _maxValue;

        analog_sensor_normval_t scale(analog_sensor_val_t val, analog_sensor_normval_t min, analog_sensor_normval_t max)
        {
            float factor = (max-min)/(_maxValue - _minValue);
            return factor * (_alignment==SENSOR_ALIGNMENT_PROPORTIONAL? (val-_minValue) : (_maxValue - val)) + min;
        }
    public:

        HardwareAnalogSensor(AnalogIn *input)
        {
            _input = input;
        }

        virtual void setMinValue(analog_sensor_val_t val)
        {
            _minValue = val;
        }

        virtual void setMaxValue(analog_sensor_val_t val)
        {
            _maxValue = val;
        }
        
        virtual void setAlignment(analog_sensor_alignment_t alignment)
        {
            _alignment = alignment;
        }
        
        virtual analog_sensor_val_t getRawValue()
        {
            return _input->read_u16();
        }
        
        virtual analog_sensor_normval_t getNormalizedValue() 
        {
            return scale(getRawValue(),0,1);
        }
        
};

#endif