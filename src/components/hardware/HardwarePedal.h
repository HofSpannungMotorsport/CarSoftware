#ifndef HARDWAREPEDAL_H
#define HARDWAREPEDAL_H

#include "mbed.h"
#include "../interface/IPedal.h"
#include "../hardware/HardwareAnalogSensor.h"

#define STD_MAX_DEVIANCE 0.10 // 10%
#define STD_MAX_DEVIANCE_TIME 100 // 100ms

#define STD_MAPPED_BOUNDARY_PERCENTAGE 0.10 // 10%
#define STD_MAX_OUT_OF_BOUNDARY_TIME 100 // 100ms

#define STD_CALIBRATION_REFRESH_TIME 0.030 // 30ms
#define STD_CALIBRATION_MIN_DEVIANCE 500 // Raw analog
#define STD_CALIBRATION_MAX_DEVIANCE 10000 // Raw analog

#define STD_ANALOG_LOWER_BOUNDARY   655 // uint16_t min ->     0
#define STD_ANALOG_UPPER_BOUNDARY 64880 // uint16_t max -> 65535

#define STD_PEDAL_THRESHHOLD 0.10 // 10%

class HardwarePedal : public IPedal {
    public:
        HardwarePedal(PinName inputPin)
            : _pin1(inputPin), _pin2(inputPin){
            _secondSensor = false;
            _init();
        }

        HardwarePedal(PinName inputPin, can_component_t componentId)
            : HardwarePedal(inputPin) {
            _componentId = componentId;
        }

        HardwarePedal(PinName inputPin1, PinName inputPin2)
            : _pin1(inputPin1), _pin2(inputPin2) {
            _secondSensor = true;
            _init();
        }

        HardwarePedal(PinName inputPin1, PinName inputPin2, can_component_t componentId)
            : HardwarePedal(inputPin1, inputPin2) {
            _componentId = componentId;
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

        virtual pedal_status_t getStatus() {
            pedal_error_type_t analogSensorError = ANALOG_SENSOR_ERROR;

            if (_pin1.getStatus() > 0) {
                _status |= analogSensorError;
            }
            
            if (_secondSensor) {
                if (_pin2.getStatus() > 0) {
                    _status |= analogSensorError;
                }
            }

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
            if (calibrationStatus != _calibrationStatus) {
                if (calibrationStatus == CURRENTLY_CALIBRATING) {
                    _beginCalibration();
                } else if (calibrationStatus == CURRENTLY_NOT_CALIBRATING) {
                    _endCalibration();
                }
                _calibrationStatus = calibrationStatus;
            }
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

        virtual void setStatus(pedal_status_t status) {
            // No implementation needed
        }
        virtual void setValue(pedal_value_t value) {
            // No impemantation needed
        }

    protected:
        HardwareAnalogSensor _pin1;
        HardwareAnalogSensor _pin2;
        pedal_sensor_type_t _pin1Proportionality = DIRECT_PROPORTIONAL,
                            _pin2Proportionality = DIRECT_PROPORTIONAL;
        bool _secondSensor;
        pedal_status_t _status = 0;
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
            } sensor1;

            struct sensor2 {
                analog_sensor_raw_t min = 0, max = 0;
            } sensor2;
        } _calibration;

        struct _deviance {
            float max = STD_MAX_DEVIANCE;
            uint16_t maxTime = STD_MAX_DEVIANCE_TIME;
            Timer timer = Timer();
            bool timerStarted = false;
        } _deviance;

        void _init() {
            _telegramTypeId = PEDAL;
            _objectType = HARDWARE_OBJECT;

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
        }

        void _calibrationLoop() {
            analog_sensor_raw_t currentValue = _pin1.getRawValue();
            if (currentValue < _calibration.sensor1.min)
                _calibration.sensor1.min = currentValue;
            if (currentValue > _calibration.sensor1.max)
                _calibration.sensor1.max = currentValue;

            if (_secondSensor) {
                currentValue = _pin2.getRawValue();
                if (currentValue < _calibration.sensor2.min)
                    _calibration.sensor2.min = currentValue;
                if (currentValue > _calibration.sensor2.max)
                    _calibration.sensor2.max = currentValue;
            }
        }

        virtual void _beginCalibration() {
            analog_sensor_raw_t sensorValue = _pin1.getRawValue();
            _calibration.sensor1.min = sensorValue;
            _calibration.sensor1.max = sensorValue;

            if (_secondSensor) {
                sensorValue = _pin2.getRawValue();
                _calibration.sensor2.min = sensorValue;
                _calibration.sensor2.max = sensorValue;
            }

            _calibrationTicker.attach(callback(this, &HardwarePedal::_calibrationLoop), _calibrationRefreshTime);
        }

        virtual void _endCalibration() {
            _calibrationTicker.detach();
            bool devianceTooLow = false;
            bool devianceTooHigh = false;

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