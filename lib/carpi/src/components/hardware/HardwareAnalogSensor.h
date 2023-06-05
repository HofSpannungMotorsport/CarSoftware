#ifndef HARDWARE_ANALOGSENSOR_H
#define HARDWARE_ANALOGSENSOR_H

#include "../interface/IAnalogSensor.h"

#define STD_OUT_OF_BOUNDARY_TIME_LIMIT 99
#define STD_OUT_OF_BOUNDARY_TIME_LIMIT_RAW 89

class HardwareAnalogSensor : public IAnalogSensor
{
public:
    HardwareAnalogSensor(PinName pin)
        : _pin(pin) {}

    virtual bool setMapping(analog_sensor_raw_t minIn, analog_sensor_raw_t maxIn, analog_sensor_t minOut, analog_sensor_t maxOut)
    {
        if ((minIn == maxIn) || (minOut == maxOut))
            return false;

        _map.from.min = minIn;
        _map.from.max = maxIn;

        _map.to.min = minOut;
        _map.to.max = maxOut;

        _map.set = true;

        return true;
    }

    virtual bool getMapping(analog_sensor_raw_t &minIn, analog_sensor_raw_t &maxIn, analog_sensor_raw_t &minOut, analog_sensor_raw_t &maxOut)
    {
        if (!_map.set)
            return false;

        minIn = _map.from.min;
        maxIn = _map.from.max;
        minOut = _map.to.min;
        maxOut = _map.to.max;

        return true;
    }

    virtual bool setRawBoundary(analog_sensor_raw_t lowerEnd, analog_sensor_raw_t upperEnd)
    {
        if (upperEnd > lowerEnd)
        {
            _boundary.raw.upperEnd = upperEnd;
            _boundary.raw.lowerEnd = lowerEnd;
        }
        else if (upperEnd < lowerEnd)
        {
            _boundary.raw.upperEnd = lowerEnd;
            _boundary.raw.lowerEnd = upperEnd;
        }
        else
            return false;

        return true;
    }

    virtual void setRawBoundaryOutTime(uint16_t time)
    {
        _boundary.raw.outTimerLimit = time;
    }

    virtual void setBoundary(analog_sensor_boundary_t boundaryPercentage)
    {
        _boundary.percentage = boundaryPercentage;
    }

    virtual void setBoundaryOutTime(uint16_t time)
    {
        _boundary.outTimerLimit = time;
    }

    virtual analog_sensor_raw_t getRawValue()
    {
        analog_sensor_raw_t sensorValue = _pin.read_u16();
        analog_sensor_raw_t returnValue = sensorValue;

        bool underBoundary = false;
        bool overBoundary = false;

        if (sensorValue < _boundary.raw.lowerEnd)
        {
            underBoundary = true;
        }

        if (sensorValue > _boundary.raw.upperEnd)
        {
            overBoundary = true;
        }

        if (underBoundary || overBoundary)
        {
            if (_boundary.raw.outTimerStarted)
            {
                if (_boundary.raw.outTimer.read_ms() > _boundary.raw.outTimerLimit)
                {
                    analog_sensor_error_type_t outOfBoundaryError = OUT_OF_BOUNDARY;
                    uint8_t error = outOfBoundaryError;

                    if (underBoundary)
                        error |= DISCONNECTED;

                    if (overBoundary)
                        error |= SHORT_CIRCUIT;

                    _status |= error;
                    returnValue = 0;
                }
                else
                {
                    returnValue = _last.raw;
                }
            }
            else
            {
                _restartTimer(_boundary.raw.outTimer);
                _boundary.raw.outTimerStarted = true;
                returnValue = _last.raw;
            }
        }
        else
        {
            if (_boundary.raw.outTimerStarted)
            {
                _boundary.raw.outTimerStarted = false;
                _boundary.raw.outTimer.stop();
            }
        }

        _last.raw = returnValue;
        return returnValue;
    }

    virtual analog_sensor_t getValue()
    {
        if (!_map.set)
            return 0;

        analog_sensor_t mappedValue = _getMapped(getRawValue());
        analog_sensor_t returnValue = mappedValue;

        bool outOfBoundary = false;

        // Much code, less functionality. Check if out of boundary.
        // At first, look if directly proportional or improportional
        if (_map.to.min < _map.to.max)
        {
            analog_sensor_t maxDeviance = _boundary.percentage * (_map.to.max - _map.to.min);
            // Then, check if mappedValue is bigger then the given map
            if (mappedValue < _map.to.min)
            {
                // Set Value to min/max...
                returnValue = _map.to.min;

                // Look if it is outside of our boundary
                if (mappedValue < (_map.to.min - maxDeviance))
                    // If is not in our boundary, save an error
                    outOfBoundary = true;
            }

            // And now the same again, just for the other cases...
            if (mappedValue > _map.to.max)
            {
                returnValue = _map.to.max;
                // if (mappedValue > (_map.to.max + maxDeviance))
                //   outOfBoundary = true;
            }
        }
        else if (_map.to.min > _map.to.max)
        {
            analog_sensor_t maxDeviance = _boundary.percentage * (_map.to.min - _map.to.max);
            if (mappedValue < _map.to.max)
            {
                returnValue = _map.to.max;
                if (mappedValue < (_map.to.max - maxDeviance))
                    outOfBoundary = true;
            }

            if (mappedValue > _map.to.min)
            {
                returnValue = _map.to.min;
                if (mappedValue > (_map.to.min + maxDeviance))
                    outOfBoundary = true;
            }
        }
        else
        {
            // If the boundary check failed, produce an error
            analog_sensor_error_type_t boundaryCheckFailError = BOUNDARY_CHECK_FAIL;
            _status |= boundaryCheckFailError;
            returnValue = 0;
        }

        // If the mappedValue is outside of our boundary, produce an error if the error persists too long
        if (outOfBoundary)
        {
            if (_boundary.outTimerStarted)
            {
                if (_boundary.outTimer.read_ms() > _boundary.outTimerLimit)
                {
                    analog_sensor_error_type_t outOfBoundaryError = OUT_OF_BOUNDARY;
                    _status |= outOfBoundaryError;
                    returnValue = 0;
                }
                else
                {
                    returnValue = _last.normal;
                }
            }
            else
            {
                _restartTimer(_boundary.outTimer);
                _boundary.outTimerStarted = true;
                returnValue = _last.normal;
            }
        }
        else
        {
            if (_boundary.outTimerStarted)
            {
                _boundary.outTimerStarted = false;
                _boundary.outTimer.stop();
            }
        }

        // If everything OK, return mappedValue/returnValue
        _last.normal = returnValue;
        return returnValue;
    }

    virtual analog_sensor_status_t getStatus()
    {
        return _status;
    }

    void reset()
    {
        _map.set = false;
        _status = 0;
    }

protected:
    AnalogIn _pin;
    analog_sensor_status_t _status = 0;

    struct _last
    {
        analog_sensor_t normal = 0;
        analog_sensor_raw_t raw = 0;
    } _last;

    struct _map
    {
        struct from
        {
            analog_sensor_raw_t min = 0, max = 0;
        } from;

        struct to
        {
            analog_sensor_t min = 0, max = 0;
        } to;

        bool set = false;
    } _map;

    struct _boundary
    {
        analog_sensor_boundary_t percentage = 0.001;

        Timer outTimer;
        bool outTimerStarted = false;
        uint16_t outTimerLimit = STD_OUT_OF_BOUNDARY_TIME_LIMIT;

        struct raw
        {
            analog_sensor_raw_t lowerEnd = 66, upperEnd = 65469;
            Timer outTimer;
            bool outTimerStarted = false;
            uint16_t outTimerLimit = STD_OUT_OF_BOUNDARY_TIME_LIMIT_RAW;
        } raw;
    } _boundary;

    void _restartTimer(Timer &timer)
    {
        timer.stop();
        timer.reset();
        timer.start();
    }

    analog_sensor_t _getMapped(analog_sensor_raw_t val)
    {
        if (_map.set)
        {
            return ((analog_sensor_t)val - (analog_sensor_t)_map.from.min) * (_map.to.max - _map.to.min) / ((analog_sensor_t)_map.from.max - (analog_sensor_t)_map.from.min) + _map.to.min;
        }
        else
            return 0;
    }
};

#endif