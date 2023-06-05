#ifndef HARDWAREPEDAL_H
#define HARDWAREPEDAL_H

#include "platform/CircularBuffer.h"
#include "../interface/IPedal.h"
#include "../hardware/HardwareAnalogSensor.h"

// Hardcoded Calibration. Can be overwritten by recalibration if any fault
#define STD_GAS_1_MIN 2000
#define STD_GAS_1_MAX 51700

#define STD_GAS_2_MIN 2000
#define STD_GAS_2_MAX 51816

#define STD_BRAKE_MIN 41800
#define STD_BRAKE_MAX 51700

// Devinance Settings
#define STD_MAX_DEVIANCE 0.35     // 10%
#define STD_MAX_DEVIANCE_TIME 200 // 100ms

#define STD_MAPPED_BOUNDARY_PERCENTAGE 0.25 // 10%
#define STD_MAX_OUT_OF_BOUNDARY_TIME 200    // 100ms [ahh... should be implemented, huh?][il]

#define STD_CALIBRATION_REFRESH_TIME 0.010    // 10ms
#define STD_CALIBRATION_MIN_DEVIANCE 200      // Raw analog
#define STD_CALIBRATION_MAX_DEVIANCE 65535    // Raw analog
#define STD_CALIBRATION_SAMPLE_BUFFER_SIZE 20 // How many values should be combined during calibration to get the fu***** deviance away

#define STD_ANALOG_LOWER_BOUNDARY 0     // uint16_t min ->     0
#define STD_ANALOG_UPPER_BOUNDARY 65535 // uint16_t max -> 65535

#define STD_PEDAL_THRESHHOLD 0.15 // 15%

struct pedal_calibration_data_t
{
    uint16_t min[2] = {0, 0}, max[2] = {0, 0};
    bool secondSensor = false;
};

struct two_raw_values_t
{
    uint16_t a, b;

    two_raw_values_t(uint16_t _a, uint16_t _b) : a(_a), b(_b) {}
};

class HardwarePedal : public IPedal
{
public:
    HardwarePedal(PinName inputPin)
        : _pin1(inputPin), _pin2(inputPin)
    {
        _secondSensor = false;
        _init();
    }

    HardwarePedal(PinName inputPin, id_sub_component_t componentSubId)
        : HardwarePedal(inputPin)
    {
        setComponentSubId(componentSubId);
    }

    HardwarePedal(PinName inputPin, id_sub_component_t componentSubId, uint16_t min, uint16_t max)
        : HardwarePedal(inputPin, componentSubId)
    {
        _pin1.setMapping(min, max, 0.0, 1.0);
        _calibrationSet = true;
        _ready = true;
    }

    HardwarePedal(PinName inputPin, id_sub_component_t componentSubId, uint16_t min, uint16_t max, uint16_t minDeadzone, uint16_t maxDeadzone)
        : HardwarePedal(inputPin, componentSubId, min, max)
    {
        _minDeadzone = minDeadzone;
        _maxDeadzone = maxDeadzone;
    }

    HardwarePedal(PinName inputPin1, PinName inputPin2) : _pin1(inputPin1), _pin2(inputPin2)
    {
        _secondSensor = true;
        _init();
    }

    HardwarePedal(PinName inputPin1, PinName inputPin2, id_sub_component_t componentSubId)
        : HardwarePedal(inputPin1, inputPin2)
    {
        setComponentSubId(componentSubId);
    }

    HardwarePedal(PinName inputPin1, PinName inputPin2, id_sub_component_t componentSubId, uint16_t minPin1, uint16_t maxPin1, uint16_t minPin2, uint16_t maxPin2)
        : HardwarePedal(inputPin1, inputPin2, componentSubId)
    {
        _pin1.setMapping(minPin1, maxPin1, 0.0, 1.0);
        _pin2.setMapping(minPin2, maxPin2, 0.0, 1.0);
        _calibrationSet = true;
        _ready = true;
    }

    HardwarePedal(PinName inputPin1, PinName inputPin2, id_sub_component_t componentSubId, uint16_t minPin1, uint16_t maxPin1, uint16_t minPin2, uint16_t maxPin2, uint16_t minDeadzone, uint16_t maxDeadzone)
        : HardwarePedal(inputPin1, inputPin2, componentSubId, minPin1, maxPin1, minPin2, maxPin2)
    {
        _minDeadzone = minDeadzone;
        _maxDeadzone = maxDeadzone;
    }

    virtual void setProportionality(pedal_sensor_type_t proportionality, uint16_t sensorNumber = 0)
    {
        if (proportionality == DIRECT_PROPORTIONAL || proportionality == INDIRECT_PROPORTIONAL)
        {
            if (sensorNumber == 0)
            {
                _pin1Proportionality = proportionality;
            }
            else if (sensorNumber == 1)
            {
                _pin2Proportionality = proportionality;
            }
        }
    }

    virtual pedal_status_t getStatus()
    {
        _status |= (_pin1.getStatus() << 4);
        _status |= (_pin2.getStatus() << 4);

        return _status;
    }

    virtual pedal_value_t getValue()
    {
        if (_ready)
        {
            pedal_value_t returnValue = _pin1.getValue();
            bool devianceTooHigh = false;

            if (_secondSensor)
            {
                pedal_value_t secondValue = _pin2.getValue();

                pedal_value_t testValue = abs(returnValue - secondValue);
                if (testValue >= _deviance.max)
                    devianceTooHigh = true;

                if (devianceTooHigh)
                {
                    returnValue = _last;
                    if (_deviance.timerStarted)
                    {
                        if (_deviance.timer.read_ms() > _deviance.maxTime)
                        {
                            pedal_error_type_t sensorDevianceTooHighError = SENSOR_DEVIANCE_TOO_HIGH;
                            _status |= sensorDevianceTooHighError;

                            returnValue = 0;
                        }
                    }
                    else
                    {
                        _restartTimer(_deviance.timer);
                        _deviance.timerStarted = true;
                    }
                }
                else
                {
                    returnValue = (returnValue + secondValue) / 2;

                    if (_deviance.timerStarted)
                    {
                        _deviance.timer.stop();
                        _deviance.timerStarted = false;
                    }
                }
            }

            // Add Threshhold
            returnValue = (returnValue - _pedalThreshhold) * 1 / (1 - _pedalThreshhold);

            if (returnValue < 0)
                returnValue = 0;

            _last = returnValue;
            return returnValue;
        }

        return 0;
    }

    two_raw_values_t
    getRaw()
    {
        if (_secondSensor)
        {
            return two_raw_values_t(_pin1.getRawValue(), _pin2.getRawValue());
        }

        return two_raw_values_t(_pin1.getRawValue(), 0);
    }

    virtual void setCalibrationStatus(pedal_calibration_t calibrationStatus)
    {
        if (calibrationStatus == CURRENTLY_CALIBRATING)
        {
            _beginCalibration();
        }
        else if (calibrationStatus == CURRENTLY_NOT_CALIBRATING)
        {
            _endCalibration();
        }
        _calibrationStatus = calibrationStatus;
    }

    virtual pedal_calibration_t getCalibrationStatus()
    {
        return _calibrationStatus;
    }

    virtual bool getCalibration(pedal_calibration_data_t &pedalCalibration)
    {
        if (!_calibration.sensor1.initPointSet)
            return false;

        if (_secondSensor)
        {
            if (!_calibration.sensor2.initPointSet)
                return false;
        }

        pedalCalibration.secondSensor = _secondSensor;

        uint16_t min = 0, max = 0, temp1, temp2;

        _pin1.getMapping(min, max, temp1, temp2);
        pedalCalibration.min[0] = min;
        pedalCalibration.max[0] = max;

        if (_secondSensor)
        {
            _pin2.getMapping(min, max, temp1, temp2);
            pedalCalibration.min[1] = min;
            pedalCalibration.max[1] = max;
        }

        return true;
    }

    virtual void setMaxDeviance(pedal_value_t deviance)
    {
        _deviance.max = deviance;
    }

    virtual void setMaxDevianceTime(uint16_t time)
    {
        _deviance.maxTime = time;
    }

    virtual message_build_result_t buildMessage(CarMessage &carMessage)
    {
        car_sub_message_t subMessage;

        subMessage.length = 3;

        subMessage.data[0] = this->getStatus();

        // change line below when type of pedal_value_t changes
        float pedalValueFloat = this->getValue();
        uint16_t pedalValue = ((float)pedalValueFloat * 65535);

#ifdef PEDAL_MESSAGE_HANDLER_DEBUG
        pcSerial.printf("[HardwarePedal]@buildMessage: HardwareObject (float)pedalValue: %.5f\t(uint16_t)pedalValue: %i\t", pedalValueFloat, pedalValue);
#endif

        subMessage.data[1] = pedalValue & 0xFF;
        subMessage.data[2] = (pedalValue >> 8) & 0xFF;

#ifdef PEDAL_MESSAGE_HANDLER_DEBUG
        pcSerial.printf("msg.data[1]: 0x%x\tmsg.data[2]: 0x%x\n", subMessage.data[1], subMessage.data[2]);
#endif

        carMessage.addSubMessage(subMessage);

        return MESSAGE_BUILD_OK;
    }

    virtual message_parse_result_t parseMessage(CarMessage &carMessage)
    {
        message_parse_result_t result = MESSAGE_PARSE_OK;
        for (car_sub_message_t &subMessage : carMessage.subMessages)
        {
            if (subMessage.length != 1) // not a valid message
                result = MESSAGE_PARSE_ERROR;

            uint8_t gotValue = subMessage.data[0];
            pedal_calibration_t calibrationStatus = CURRENTLY_NOT_CALIBRATING;

            if (gotValue == 1)
            {
                calibrationStatus = CURRENTLY_NOT_CALIBRATING;
            }
            else if (gotValue == 2)
            {
                calibrationStatus = CURRENTLY_CALIBRATING;
            }

            this->setCalibrationStatus(calibrationStatus);

#ifdef PEDAL_MESSAGE_HANDLER_DEBUG
            pcSerial.printf("[HardwarePedal]@parseMessage: SoftwareObject calibrationStatus: 0x%x\tmsg.data[0]: 0x%x\tgotValue: %i\n", calibrationStatus, subMessage.data[0], gotValue);
#endif
        }

        return result;
    }

    float getValueAge()
    {
        return 0;
    }

    void resetAge() {}

protected:
    HardwareAnalogSensor _pin1;
    HardwareAnalogSensor _pin2;
    pedal_sensor_type_t _pin1Proportionality = DIRECT_PROPORTIONAL,
                        _pin2Proportionality = DIRECT_PROPORTIONAL;
    bool _secondSensor;
    pedal_status_t _status = 0;
    bool _ready = false;
    pedal_value_t _last = 0;
    bool _calibrationSet = false;
    uint16_t _minDeadzone = 0;
    uint16_t _maxDeadzone = 0;

    float _calibrationRefreshTime = STD_CALIBRATION_REFRESH_TIME;
    uint16_t _calibrationMinDeviance = STD_CALIBRATION_MIN_DEVIANCE;
    uint16_t _calibrationMaxDeviance = STD_CALIBRATION_MAX_DEVIANCE;

    analog_sensor_raw_t _analogLowerBoundary = STD_ANALOG_LOWER_BOUNDARY;
    analog_sensor_raw_t _analogUpperBoundary = STD_ANALOG_UPPER_BOUNDARY;

    float _pedalThreshhold = STD_PEDAL_THRESHHOLD;

    Ticker _calibrationTicker;
    pedal_calibration_t _calibrationStatus = CURRENTLY_NOT_CALIBRATING;

    struct _calibration
    {
        struct sensor1
        {
            analog_sensor_raw_t min = 0, max = 0;
            bool initPointSet = false;
            CircularBuffer<analog_sensor_raw_t, STD_CALIBRATION_SAMPLE_BUFFER_SIZE> buffer;
        } sensor1;

        struct sensor2
        {
            analog_sensor_raw_t min = 0, max = 0;
            bool initPointSet = false;
            CircularBuffer<analog_sensor_raw_t, STD_CALIBRATION_SAMPLE_BUFFER_SIZE> buffer;
        } sensor2;
    } _calibration;

    struct _deviance
    {
        float max = STD_MAX_DEVIANCE;
        uint16_t maxTime = STD_MAX_DEVIANCE_TIME;
        Timer timer;
        bool timerStarted = false;
    } _deviance;

    void _init()
    {
        setComponentType(COMPONENT_PEDAL);
        setObjectType(OBJECT_HARDWARE);

        _pin1.setRawBoundary(_analogLowerBoundary, _analogUpperBoundary);
        _pin1.setRawBoundaryOutTime(STD_MAX_OUT_OF_BOUNDARY_TIME);
        _pin1.setBoundary(STD_MAPPED_BOUNDARY_PERCENTAGE);
        _pin1.setBoundaryOutTime(STD_MAX_OUT_OF_BOUNDARY_TIME);

        if (_secondSensor)
        {
            _pin2.setRawBoundary(_analogLowerBoundary, _analogUpperBoundary);
            _pin2.setRawBoundaryOutTime(STD_MAX_OUT_OF_BOUNDARY_TIME);
            _pin2.setBoundary(STD_MAPPED_BOUNDARY_PERCENTAGE);
            _pin2.setBoundaryOutTime(STD_MAX_OUT_OF_BOUNDARY_TIME);
        }
    }

    analog_sensor_raw_t _getAverageValue(CircularBuffer<analog_sensor_raw_t, STD_CALIBRATION_SAMPLE_BUFFER_SIZE> &buffer)
    {
        uint32_t value = 0,
                 valueCount = 0;

        while (!buffer.empty())
        {
            analog_sensor_raw_t currentValue = 0; // Initializing with 0, don't want to get an compiler warning over and over again
            buffer.pop(currentValue);
            value += currentValue;
            valueCount++;
        }

        value /= valueCount;
        return value;
    }

    void _calibrationLoop()
    {
        _calibration.sensor1.buffer.push(_pin1.getRawValue());
        if (_calibration.sensor1.buffer.full())
        {
            analog_sensor_raw_t currentValue = _getAverageValue(_calibration.sensor1.buffer);

            if (_calibration.sensor1.initPointSet)
            {
                if (currentValue + _minDeadzone < _calibration.sensor1.min)
                    _calibration.sensor1.min = currentValue + _minDeadzone;
                if (currentValue + _maxDeadzone > _calibration.sensor1.max)
                    _calibration.sensor1.max = currentValue + _maxDeadzone;
            }
            else
            {
                _calibration.sensor1.min = currentValue + _minDeadzone;
                _calibration.sensor1.max = currentValue + _maxDeadzone;
                _calibration.sensor1.initPointSet = true;
            }
        }

        if (_secondSensor)
        {
            _calibration.sensor2.buffer.push(_pin2.getRawValue());
            if (_calibration.sensor2.buffer.full())
            {
                analog_sensor_raw_t currentValue = _getAverageValue(_calibration.sensor2.buffer);

                if (_calibration.sensor2.initPointSet)
                {
                    if (currentValue + _minDeadzone < _calibration.sensor2.min)
                        _calibration.sensor2.min = currentValue + _minDeadzone;
                    if (currentValue + _maxDeadzone > _calibration.sensor2.max)
                        _calibration.sensor2.max = currentValue + _maxDeadzone;
                }
                else
                {
                    _calibration.sensor2.min = currentValue + _minDeadzone;
                    _calibration.sensor2.max = currentValue + _maxDeadzone;
                    _calibration.sensor2.initPointSet = true;
                }
            }
        }
    }

    virtual void _beginCalibration()
    {
        _ready = false;

        // Reset Status and deviance Timer to continue after calibration without an preveouse error
        _status = 0;
        _last = 0;
        if (_deviance.timerStarted)
        {
            _deviance.timer.stop();
            _deviance.timerStarted = false;
        }
        _pin1.reset();
        if (_secondSensor)
        {
            _pin2.reset();
        }

        _calibration.sensor1.initPointSet = false;
        _calibration.sensor1.buffer.reset();

        if (_secondSensor)
        {
            _calibration.sensor2.initPointSet = false;
            _calibration.sensor2.buffer.reset();
        }

        _calibrationTicker.attach(callback(this, &HardwarePedal::_calibrationLoop), _calibrationRefreshTime);
    }

    virtual void _endCalibration()
    {
        _calibrationTicker.detach();
        bool devianceTooLow = false;
        bool devianceTooHigh = false;

        if (!_calibration.sensor1.initPointSet)
        {
            devianceTooLow = true;

            _calibration.sensor1.min = 0;
            _calibration.sensor1.max = 0;
        }

        if (_secondSensor)
        {
            if (!_calibration.sensor2.initPointSet)
            {
                devianceTooLow = true;

                _calibration.sensor2.min = 0;
                _calibration.sensor2.max = 0;
            }
        }

        if ((_calibration.sensor1.max - _calibration.sensor1.min) >= _calibrationMinDeviance)
        {
            if ((_calibration.sensor1.max - _calibration.sensor1.min) <= _calibrationMaxDeviance)
            {
                if (_pin1Proportionality == DIRECT_PROPORTIONAL)
                {
                    _pin1.setMapping(_calibration.sensor1.min, _calibration.sensor1.max, 0.0, 1.0);
                }
                else if (_pin1Proportionality == INDIRECT_PROPORTIONAL)
                {
                    _pin1.setMapping(_calibration.sensor1.max, _calibration.sensor1.min, 0.0, 1.0);
                }
                else
                {
                    pedal_error_type_t calibrationFailedWrongConfigError = CALIBRATION_FAILED_WRONG_CONFIG;
                    _status |= calibrationFailedWrongConfigError;
                    return;
                }
            }
            else
            {
                devianceTooHigh = true;
            }
        }
        else
        {
            devianceTooLow = true;
        }

        if (_secondSensor)
        {
            if ((_calibration.sensor2.max - _calibration.sensor2.min) >= _calibrationMinDeviance)
            {
                if ((_calibration.sensor2.max - _calibration.sensor2.min) <= _calibrationMaxDeviance)
                {
                    if (_pin2Proportionality == DIRECT_PROPORTIONAL)
                    {
                        _pin2.setMapping(_calibration.sensor2.min, _calibration.sensor2.max, 0.0, 1.0);
                    }
                    else if (_pin2Proportionality == INDIRECT_PROPORTIONAL)
                    {
                        _pin2.setMapping(_calibration.sensor2.max, _calibration.sensor2.min, 0.0, 1.0);
                    }
                    else
                    {
                        pedal_error_type_t calibrationFailedWrongConfigError = CALIBRATION_FAILED_WRONG_CONFIG;
                        _status |= calibrationFailedWrongConfigError;
                        return;
                    }
                }
                else
                {
                    devianceTooHigh = true;
                }
            }
            else
            {
                devianceTooLow = true;
            }
        }

        bool maybeReady = true;

        if (devianceTooLow)
        {
            pedal_error_type_t calibrationFailedTooLowDevianceError = CALIBRATION_FAILED_TOO_LOW_DEVIANCE;
            _status |= calibrationFailedTooLowDevianceError;
            maybeReady = false;
        }

        if (devianceTooHigh)
        {
            pedal_error_type_t calibrationFailedTooHighDevianceError = CALIBRATION_FAILED_TOO_HIGH_DEVIANCE;
            _status |= calibrationFailedTooHighDevianceError;
            maybeReady = false;
        }

        _ready = maybeReady;

        if (_ready)
            _calibrationSet = true;
    }

    void _restartTimer(Timer &timer)
    {
        timer.stop();
        timer.reset();
        timer.start();
    }

    // Uff, too much text! Thanks error checking...
};

#endif