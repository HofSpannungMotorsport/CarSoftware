#ifndef HARDWAREPEDAL_H
#define HARDWAREPEDAL_H

#include "platform/CircularBuffer.h"
#include "../interface/IPedal.h"
#include "../hardware/HardwareAnalogSensor.h"

#define STD_MAX_DEVIANCE 0.10 // 10%
#define STD_MAX_DEVIANCE_TIME 100 // 100ms

#define STD_MAPPED_BOUNDARY_PERCENTAGE 0.10 // 10%
#define STD_MAX_OUT_OF_BOUNDARY_TIME 100 // 100ms

#define STD_CALIBRATION_REFRESH_TIME        0.030 // 30ms
#define STD_CALIBRATION_MIN_DEVIANCE        500 // Raw analog
#define STD_CALIBRATION_MAX_DEVIANCE        30000 // Raw analog
#define STD_CALIBRATION_SAMPLE_BUFFER_SIZE  20 // How many values should be combined during calibration to get the fu***** deviance away

#define STD_ANALOG_LOWER_BOUNDARY   655 // uint16_t min ->     0
#define STD_ANALOG_UPPER_BOUNDARY 64880 // uint16_t max -> 65535

#define STD_PEDAL_THRESHHOLD 0.15 // 15%

class HardwarePedal : public IPedal {
    public:
        HardwarePedal(PinName inputPin, id_sub_component_t componentSubId)
            :  _pin1(inputPin), _pin2(inputPin) {
            _secondSensor = false;
            _init();
            setComponentSubId(componentSubId);
        }

        HardwarePedal(PinName inputPin1, PinName inputPin2, id_sub_component_t componentSubId)
            : _pin1(inputPin1), _pin2(inputPin2) {
            _secondSensor = true;
            _init();
            setComponentSubId(componentSubId);
        }

        virtual void setProportionality(pedal_sensor_type_t proportionality, uint16_t sensorNumber = 0) {
            if (proportionality == DIRECT_PROPORTIONAL || proportionality == INDIRECT_PROPORTIONAL) {
                if (sensorNumber == 0) {
                    _pin1Proportionality = proportionality;
                } else if (sensorNumber == 1) {
                    _pin2Proportionality = proportionality;
                }
            }
        }

        virtual status_t getStatus() {
            _status |= (_pin1.getStatus() << 4);
            _status |= (_pin2.getStatus() << 4);

            return _status;
        }

        virtual pedal_value_t getValue() {
            if (_ready) {
                pedal_value_t returnValue = _pin1.getValue();
                bool devianceTooHigh = false;

                if (_secondSensor) {
                    pedal_value_t secondValue = _pin2.getValue();

                    if (returnValue > secondValue) {
                        pedal_value_t testValue = returnValue - secondValue;
                        if (testValue >= _deviance.max)
                            devianceTooHigh = true;
                    } else if (returnValue < secondValue) {
                        pedal_value_t testValue = secondValue - returnValue;
                        if (testValue >= _deviance.max)
                            devianceTooHigh = true;
                    }

                    if (devianceTooHigh) {
                        returnValue = _last;
                        if (_deviance.timerStarted) {
                            if (_deviance.timer.read_ms() > _deviance.maxTime) {
                                pedal_error_type_t sensorDevianceTooHighError = SENSOR_DEVIANCE_TOO_HIGH;
                                _status |= sensorDevianceTooHighError;

                                returnValue = 0;
                            }
                        } else {
                            _restartTimer(_deviance.timer);
                            _deviance.timerStarted = true;
                        }
                    } else {
                        returnValue = (returnValue + secondValue) / 2;

                        if (_deviance.timerStarted) {
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

        virtual void setCalibrationStatus(pedal_calibration_t calibrationStatus) {
            if (calibrationStatus == CURRENTLY_CALIBRATING) {
                _beginCalibration();
            } else if (calibrationStatus == CURRENTLY_NOT_CALIBRATING) {
                _endCalibration();
            }
            _calibrationStatus = calibrationStatus;
        }

        virtual pedal_calibration_t getCalibrationStatus() {
            return _calibrationStatus;
        }

        virtual void setMaxDeviance(pedal_value_t deviance) {
            _deviance.max = deviance;
        }

        virtual void setMaxDevianceTime(uint16_t time) {
            _deviance.maxTime = time;
        }

        virtual void setStatus(status_t status) {
            // No implementation needed
        }
        virtual void setValue(pedal_value_t value) {
            // No impemantation needed
        }

        virtual void receive(CarMessage &carMessage) {
            for (car_sub_message_t &subMessage : carMessage.subMessages) {
                switch (subMessage.data[0]) {
                    case PEDAL_MESSAGE_COMMAND_SET_CALIBRATION_STATUS:
                        setCalibrationStatus((pedal_calibration_t)subMessage.data[1]);
                        break;
                    
                    case PEDAL_MESSAGE_COMMAND_SET_PROPORTIONALITY_SENSOR_1:
                        setProportionality((pedal_sensor_type_t)subMessage.data[1], 0);
                        break;
                    
                    case PEDAL_MESSAGE_COMMAND_SET_PROPORTIONALITY_SENSOR_2:
                        setProportionality((pedal_sensor_type_t)subMessage.data[1], 1);
                        break;
                }
            }
        }

    protected:
        Ticker _pedalPositionTicker;
        Ticker _statusTicker;

        HardwareAnalogSensor _pin1;
        HardwareAnalogSensor _pin2;
        pedal_sensor_type_t _pin1Proportionality = DIRECT_PROPORTIONAL,
                            _pin2Proportionality = DIRECT_PROPORTIONAL;
        bool _secondSensor;
        status_t _status = 0;
        bool _ready = false;
        pedal_value_t _last = 0;

        float _calibrationRefreshTime = STD_CALIBRATION_REFRESH_TIME;
        uint16_t _calibrationMinDeviance = STD_CALIBRATION_MIN_DEVIANCE;
        uint16_t _calibrationMaxDeviance = STD_CALIBRATION_MAX_DEVIANCE;

        analog_sensor_raw_t _analogLowerBoundary = STD_ANALOG_LOWER_BOUNDARY;
        analog_sensor_raw_t _analogUpperBoundary = STD_ANALOG_UPPER_BOUNDARY;

        float _pedalThreshhold = STD_PEDAL_THRESHHOLD;

        Ticker _calibrationTicker;
        pedal_calibration_t _calibrationStatus = CURRENTLY_NOT_CALIBRATING;

        struct _calibration {
            struct sensor1 {
                analog_sensor_raw_t min = 0, max = 0;
                bool initPointSet = false;
                CircularBuffer<analog_sensor_raw_t, STD_CALIBRATION_SAMPLE_BUFFER_SIZE> buffer;
            } sensor1;

            struct sensor2 {
                analog_sensor_raw_t min = 0, max = 0;
                bool initPointSet = false;
                CircularBuffer<analog_sensor_raw_t, STD_CALIBRATION_SAMPLE_BUFFER_SIZE> buffer;
            } sensor2;
        } _calibration;

        struct _deviance {
            float max = STD_MAX_DEVIANCE;
            uint16_t maxTime = STD_MAX_DEVIANCE_TIME;
            Timer timer;
            bool timerStarted = false;
        } _deviance;

        void _updatePedalPosition() {
            if (_syncerAttached) {
                uint16_t pedalValue = ((float)getValue() * 65535);

                _sendCommand(PEDAL_MESSAGE_COMMAND_SET_VALUE, pedalValue & 0xFF, (pedalValue >> 8) & 0xFF, SEND_PRIORITY_PEDAL, STD_PEDAL_MESSAGE_TIMEOUT, IS_DROPABLE);
            }
        }

        void _updateStatus() {
            if (_syncerAttached) {
                _sendCommand(PEDAL_MESSAGE_COMMAND_SET_STATUS, getStatus(), SEND_PRIORITY_PEDAL, STD_PEDAL_MESSAGE_TIMEOUT, IS_NOT_DROPABLE);
            }
        }

        void _init() {
            setComponentType(COMPONENT_PEDAL);
            setObjectType(OBJECT_HARDWARE);

            _pin1.setRawBoundary(_analogLowerBoundary, _analogUpperBoundary);
            _pin1.setRawBoundaryOutTime(STD_MAX_DEVIANCE_TIME);
            _pin1.setBoundary(STD_MAPPED_BOUNDARY_PERCENTAGE);
            _pin1.setBoundaryOutTime(STD_MAX_OUT_OF_BOUNDARY_TIME);

            if (_secondSensor) {
                _pin2.setRawBoundary(_analogLowerBoundary, _analogUpperBoundary);
                _pin2.setRawBoundaryOutTime(STD_MAX_DEVIANCE_TIME);
                _pin2.setBoundary(STD_MAPPED_BOUNDARY_PERCENTAGE);
                _pin2.setBoundaryOutTime(STD_MAX_OUT_OF_BOUNDARY_TIME);
            }

            _pedalPositionTicker.attach(callback(this, &HardwarePedal::_updatePedalPosition), 1.0/(float)STD_PEDAL_VALUE_REFRESH_RATE);
            _statusTicker.attach(callback(this, &HardwarePedal::_updateStatus), STD_PEDAL_STATUS_REFRESH_TIME);
        }

        analog_sensor_raw_t _getAverageValue(CircularBuffer<analog_sensor_raw_t, STD_CALIBRATION_SAMPLE_BUFFER_SIZE> &buffer) {
            uint32_t value = 0,
                     valueCount = 0;

            while(!buffer.empty()) {
                analog_sensor_raw_t currentValue = 0; // Initializing with 0, don't want to get an compiler warning over and over again
                buffer.pop(currentValue);
                value += currentValue;
                valueCount++;
            }

            value /= valueCount;
            return value;
        }

        void _calibrationLoop() {
            _calibration.sensor1.buffer.push(_pin1.getRawValue());
            if (_calibration.sensor1.buffer.full()) {
                analog_sensor_raw_t currentValue = _getAverageValue(_calibration.sensor1.buffer);

                if (_calibration.sensor1.initPointSet) {
                    if (currentValue < _calibration.sensor1.min)
                        _calibration.sensor1.min = currentValue;
                    if (currentValue > _calibration.sensor1.max)
                        _calibration.sensor1.max = currentValue;
                } else {
                    _calibration.sensor1.min = currentValue;
                    _calibration.sensor1.max = currentValue;
                    _calibration.sensor1.initPointSet = true;
                }
            }

            if (_secondSensor) {
                _calibration.sensor2.buffer.push(_pin2.getRawValue());
                if (_calibration.sensor2.buffer.full()) {
                    analog_sensor_raw_t currentValue = _getAverageValue(_calibration.sensor2.buffer);

                    if (_calibration.sensor2.initPointSet) {
                        if (currentValue < _calibration.sensor2.min)
                            _calibration.sensor2.min = currentValue;
                        if (currentValue > _calibration.sensor2.max)
                            _calibration.sensor2.max = currentValue;
                    } else {
                        _calibration.sensor2.min = currentValue;
                        _calibration.sensor2.max = currentValue;
                        _calibration.sensor2.initPointSet = true;
                    }
                }
            }
        }

        virtual void _beginCalibration() {
            _calibration.sensor1.initPointSet = false;
            _calibration.sensor1.buffer.reset();

            if (_secondSensor) {
                _calibration.sensor2.initPointSet = false;
                _calibration.sensor2.buffer.reset();
            }

            _calibrationTicker.attach(callback(this, &HardwarePedal::_calibrationLoop), _calibrationRefreshTime);
        }

        virtual void _endCalibration() {
            _calibrationTicker.detach();
            bool devianceTooLow = false;
            bool devianceTooHigh = false;

            if (!_calibration.sensor1.initPointSet) {
                devianceTooLow = true;

                _calibration.sensor1.min = 0;
                _calibration.sensor1.max = 0;
            }

            if (_secondSensor) {
                if (!_calibration.sensor2.initPointSet) {
                    devianceTooLow = true;

                    _calibration.sensor2.min = 0;
                    _calibration.sensor2.max = 0;
                }
            }

            if ((_calibration.sensor1.max - _calibration.sensor1.min) >= _calibrationMinDeviance) {
                if ((_calibration.sensor1.max - _calibration.sensor1.min) <= _calibrationMaxDeviance) {
                    if (_pin1Proportionality == DIRECT_PROPORTIONAL) {
                        _pin1.setMapping(_calibration.sensor1.min, _calibration.sensor1.max, 0.0, 1.0);
                    } else if (_pin1Proportionality == INDIRECT_PROPORTIONAL) {
                        _pin1.setMapping(_calibration.sensor1.max, _calibration.sensor1.min, 0.0, 1.0);
                    } else {
                        pedal_error_type_t calibrationFailedWrongConfigError = CALIBRATION_FAILED_WRONG_CONFIG;
                        _status |= calibrationFailedWrongConfigError;
                        return;
                    }
                } else {
                    devianceTooHigh = true;
                }
            } else {
                devianceTooLow = true;
            }

            if (_secondSensor) {
                if ((_calibration.sensor2.max - _calibration.sensor2.min) >= _calibrationMinDeviance) {
                    if ((_calibration.sensor2.max - _calibration.sensor2.min) <= _calibrationMaxDeviance) {
                        if (_pin2Proportionality == DIRECT_PROPORTIONAL) {
                            _pin2.setMapping(_calibration.sensor2.min, _calibration.sensor2.max, 0.0, 1.0);
                        } else if (_pin2Proportionality == INDIRECT_PROPORTIONAL) {
                            _pin2.setMapping(_calibration.sensor2.max, _calibration.sensor2.min, 0.0, 1.0);
                        } else {
                            pedal_error_type_t calibrationFailedWrongConfigError = CALIBRATION_FAILED_WRONG_CONFIG;
                            _status |= calibrationFailedWrongConfigError;
                            return;
                        }
                    } else {
                        devianceTooHigh = true;
                    }
                } else {
                    devianceTooLow = true;
                }
            }

            bool maybeReady = true;

            if (devianceTooLow) {
                pedal_error_type_t calibrationFailedTooLowDevianceError = CALIBRATION_FAILED_TOO_LOW_DEVIANCE;
                _status |= calibrationFailedTooLowDevianceError;
                maybeReady = false;
            }

            if (devianceTooHigh) {
                pedal_error_type_t calibrationFailedTooHighDevianceError = CALIBRATION_FAILED_TOO_HIGH_DEVIANCE;
                _status |= calibrationFailedTooHighDevianceError;
                maybeReady = false;
            }

            _ready = maybeReady;
        }

        void _restartTimer(Timer &timer) {
            timer.stop();
            timer.reset();
            timer.start();
        }

        // Uff, too much text! Thanks error checking...
};

#endif