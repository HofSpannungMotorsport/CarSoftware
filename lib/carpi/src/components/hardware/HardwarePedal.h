#ifndef HARDWAREPEDAL_H
#define HARDWAREPEDAL_H

#include "../interface/IPedal.h"
#include "../hardware/HardwareAnalogSensor.h"
#include "HardConfig.h"

struct pedal_calibration_data_t {
    pedal_calibration_data_t();

    pedal_calibration_data_t(uint16_t min0, uint16_t max0) {
        min[0] = min0;
        max[0] = max0;
        secondSensor = false;
    }
    
    pedal_calibration_data_t(uint16_t min0, uint16_t max0, uint16_t min1, uint16_t max1) {
        min[0] = min0;
        max[0] = max0;
        min[1] = min1;
        max[1] = max1;
        secondSensor = true;
    }

    uint16_t min[2] = {0,0}, max[2] = {0,0};
    bool secondSensor = false;
};

class HardwarePedal : public IPedal {
    public:
        HardwarePedal(PinName inputPin, id_sub_component_t componentSubId, IRegistry &registry)
            : _pin1(inputPin, registry), _pin2(inputPin, registry), _registry(registry) {
            setComponentSubId(componentSubId);
            _secondSensor = false;
        }

        HardwarePedal(PinName inputPin1, PinName inputPin2, id_sub_component_t componentSubId, IRegistry &registry)
            : _pin1(inputPin1, registry), _pin2(inputPin2, registry), _registry(registry) {
            setComponentSubId(componentSubId);
            _secondSensor = true;
        }

        virtual void setProportionality(pedal_sensor_type_t proportionality, uint16_t sensorNumber = 0) {
            _init();
            if (proportionality == DIRECT_PROPORTIONAL || proportionality == INDIRECT_PROPORTIONAL) {
                if (sensorNumber == 0) {
                    _pin1Proportionality = proportionality;
                } else if (sensorNumber == 1) {
                    _pin2Proportionality = proportionality;
                }
            }
        }

        virtual status_t getStatus() {
            _init();
            _status |= (_pin1.getStatus() << 4);
            _status |= (_pin2.getStatus() << 4);

            return _status;
        }

        virtual pedal_value_t getValue() {
            _init();
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
            _init();
            if (calibrationStatus == CURRENTLY_CALIBRATING) {
                _beginCalibration();
            } else if (calibrationStatus == CURRENTLY_NOT_CALIBRATING) {
                _endCalibration();
            }
            _calibrationStatus = calibrationStatus;
        }

        virtual pedal_calibration_t getCalibrationStatus() {
            _init();
            return _calibrationStatus;
        }

        virtual bool getCalibration(pedal_calibration_data_t &pedalCalibration) {
            _init();
            if (!_calibration.sensor1.initPointSet) return false;

            if (_secondSensor) {
                if (!_calibration.sensor2.initPointSet) return false;
            }

            pedalCalibration.secondSensor = _secondSensor;

            uint16_t min = 0, max = 0, temp1, temp2;

            _pin1.getMapping(min, max, temp1, temp2);
            pedalCalibration.min[0] = min;
            pedalCalibration.max[0] = max;

            if (_secondSensor) {
                _pin2.getMapping(min, max, temp1, temp2);
                pedalCalibration.min[1] = min;
                pedalCalibration.max[1] = max;
            }

            return true;
        }

        bool setCalibration(pedal_calibration_data_t &pedalCalibration) {
            _init();

            if (pedalCalibration.secondSensor != _secondSensor)
                return false;
            
            _pin1.setMapping(pedalCalibration.min[0], pedalCalibration.max[0], 0.0, 1.0);

            if (_secondSensor)
                _pin2.setMapping(pedalCalibration.min[1], pedalCalibration.max[1], 0.0, 1.0);

            _calibrationSet = true;
            _ready = true;

            return true;
        }

        virtual void setMaxDeviance(pedal_value_t deviance) {
            _init();
            _deviance.max = deviance;
        }

        virtual void setMaxDevianceTime(uint16_t time) {
            _init();
            _deviance.maxTime = time;
        }

        virtual void setStatus(status_t status) {
            // No implementation needed
        }
        virtual void setValue(pedal_value_t value) {
            // No impemantation needed
        }

        virtual void receive(CarMessage &carMessage) {
            _init();
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

        virtual void attach(Sync &syncer) {
            IPedal::attach(syncer);
            _init();

            _pedalPositionTicker.attach(callback(this, &HardwarePedal::_updatePedalPosition), 1.0/(float)STD_PEDAL_VALUE_REFRESH_RATE);
            _statusTicker.attach(callback(this, &HardwarePedal::_updateStatus), STD_PEDAL_STATUS_REFRESH_TIME);
        }

    protected:
        IRegistry &_registry;
        bool _initComplete = false;

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
        bool _calibrationSet = false;

        float _calibrationRefreshTime;
        uint16_t _calibrationMinDeviance;
        uint16_t _calibrationMaxDeviance;

        float _pedalThreshhold;

        Ticker _calibrationTicker;
        pedal_calibration_t _calibrationStatus = CURRENTLY_NOT_CALIBRATING;

        struct _calibration {
            struct sensor1 {
                analog_sensor_raw_t min = 0, max = 0;
                bool initPointSet = false;
                CircularBuffer<analog_sensor_raw_t, PEDAL_CALIBRATION_SAMPLE_BUFFER_SIZE> buffer;
            } sensor1;

            struct sensor2 {
                analog_sensor_raw_t min = 0, max = 0;
                bool initPointSet = false;
                CircularBuffer<analog_sensor_raw_t, PEDAL_CALIBRATION_SAMPLE_BUFFER_SIZE> buffer;
            } sensor2;
        } _calibration;

        struct _deviance {
            float max;
            uint16_t maxTime;
            Timer timer;
            bool timerStarted = false;
        } _deviance;

        void _updatePedalPosition() {
            if (_syncerAttached) {
                uint16_t pedalValue = ((float)getValue() * 65535);

                _sendCommand(PEDAL_MESSAGE_COMMAND_SET_VALUE, pedalValue & 0xFF, (pedalValue >> 8) & 0xFF, SEND_PRIORITY_PEDAL, IS_DROPABLE);
            }
        }

        status_t _lastSentStatus = 0;
        void _updateStatus() {
            if (_syncerAttached) {
                status_t currentStatus = getStatus();
                if(_lastSentStatus != currentStatus) {
                    _lastSentStatus = currentStatus;
                    _sendCommand(PEDAL_MESSAGE_COMMAND_SET_STATUS, currentStatus, SEND_PRIORITY_PEDAL, IS_NOT_DROPABLE);
                }
            }
        }

        void _init() {
            if (!_initComplete) {
                // Config
                setObjectType(OBJECT_HARDWARE);

                // Analog Sensor Config
                _pin1.setRawBoundary(_registry.getUInt16(STD_PEDAL_ANALOG_LOWER_BOUNDARY), _registry.getUInt16(STD_PEDAL_ANALOG_UPPER_BOUNDARY));
                _pin1.setRawBoundaryOutTime(_registry.getUInt16(STD_PEDAL_MAX_OUT_OF_BOUNDARY_TIME));
                _pin1.setBoundary(_registry.getFloat(STD_PEDAL_MAPPED_BOUNDARY_PERCENTAGE));
                _pin1.setBoundaryOutTime(_registry.getUInt16(STD_PEDAL_MAX_OUT_OF_BOUNDARY_TIME));

                if (_secondSensor) {
                    _pin2.setRawBoundary(_registry.getUInt16(STD_PEDAL_ANALOG_LOWER_BOUNDARY), _registry.getUInt16(STD_PEDAL_ANALOG_UPPER_BOUNDARY));
                    _pin2.setRawBoundaryOutTime(_registry.getUInt16(STD_PEDAL_MAX_OUT_OF_BOUNDARY_TIME));
                    _pin2.setBoundary(_registry.getFloat(STD_PEDAL_MAPPED_BOUNDARY_PERCENTAGE));
                    _pin2.setBoundaryOutTime(_registry.getUInt16(STD_PEDAL_MAX_OUT_OF_BOUNDARY_TIME));
                }

                // Deviance Config
                _deviance.max = _registry.getFloat(STD_PEDAL_MAX_DEVIANCE);
                _deviance.maxTime = _registry.getUInt16(STD_PEDAL_MAX_DEVIANCE_TIME);

                // Calibration
                _calibrationRefreshTime = _registry.getFloat(STD_PEDAL_CALIBRATION_REFRESH_TIME);
                _calibrationMinDeviance = _registry.getUInt16(STD_PEDAL_CALIBRATION_MIN_DEVIANCE);
                _calibrationMaxDeviance = _registry.getUInt16(STD_PEDAL_CALIBRATION_MAX_DEVIANCE);

                // Pedal
                _pedalThreshhold = _registry.getFloat(STD_PEDAL_THRESHHOLD);

                _initComplete = true;
            }
            
        }

        analog_sensor_raw_t _getAverageValue(CircularBuffer<analog_sensor_raw_t, PEDAL_CALIBRATION_SAMPLE_BUFFER_SIZE> &buffer) {
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
            _ready = false;

            // Reset Status and deviance Timer to continue after calibration without an preveouse error
            _status = 0;
            _last = 0;
            if (_deviance.timerStarted) {
                _deviance.timer.stop();
                _deviance.timerStarted = false;
            }
            _pin1.reset();
            if (_secondSensor) {
                _pin2.reset();
            }

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

            if (_ready) _calibrationSet = true;
        }

        void _restartTimer(Timer &timer) {
            timer.stop();
            timer.reset();
            timer.start();
        }

        // Uff, too much text! Thanks error checking...
};

#endif