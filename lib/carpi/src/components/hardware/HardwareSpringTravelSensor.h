#ifndef HARDWARE_SPRING_TRAVEL_SENSOR_H
#define HARDWARE_SPRING_TRAVEL_SENSOR_H

#include "mbed.h"

#include "../interface/ISpringTravelSensor.h"
#include "../hardware/HardwareAnalogSensor.h"


#define STD_SPRING_MAPPED_BOUNDARY_PERCENTAGE 0.10 // 10%
#define STD_SPRING_MAX_OUT_OF_BOUNDARY_TIME 100 // 100ms
#define STD_SPRING_MAX_OUT_OF_RAW_BOUNDARY_TIME 100 // 100ms

#define STD_SPRING_CALIBRATION_REFRESH_TIME        0.030 // 30ms
#define STD_SPRING_CALIBRATION_MIN_DEVIANCE        0 // Raw analog
#define STD_SPRING_CALIBRATION_MAX_DEVIANCE        65535 // Raw analog
#define STD_SPRING_CALIBRATION_SAMPLE_BUFFER_SIZE  20 // How many values should be combined during calibration to get the fu***** deviance away

#define STD_SPRING_ANALOG_LOWER_BOUNDARY   655 // uint16_t min ->     0
#define STD_SPRING_ANALOG_UPPER_BOUNDARY 64880 // uint16_t max -> 65535

class HardwareSpringTravelSensor : public ISpringTravelSensor {
    public:
        HardwareSpringTravelSensor(PinName inputPin, id_sub_component_t componentSubId)
            :  _pin(inputPin) {
            setComponentType(COMPONENT_SUSPENSION_TRAVEL);
            setObjectType(OBJECT_HARDWARE);

            _pin.setRawBoundary(_analogLowerBoundary, _analogUpperBoundary);
            _pin.setRawBoundaryOutTime(STD_SPRING_MAX_OUT_OF_RAW_BOUNDARY_TIME);
            _pin.setBoundary(STD_SPRING_MAPPED_BOUNDARY_PERCENTAGE);
            _pin.setBoundaryOutTime(STD_SPRING_MAX_OUT_OF_BOUNDARY_TIME);

            _springBothTicker.attach(callback(this, &HardwareSpringTravelSensor::_updateBoth), 1.0/(float)STD_SPRING_VALUE_REFRESH_RATE);
            _statusTicker.attach(callback(this, &HardwareSpringTravelSensor::_updateStatus), STD_SPRING_STATUS_REFRESH_TIME);

            setComponentSubId(componentSubId);
        }

        virtual void setProportionality(spring_sensor_type_t proportionality) {
            if (proportionality == DIRECT_PROPORTIONAL || proportionality == INDIRECT_PROPORTIONAL) {
                _proportionality = proportionality;
            }
        }

        virtual status_t getStatus() {
            _status |= (_pin.getStatus() << 4);

            return _status;
        }

        virtual spring_raw_t getRaw() {
            return _pin.getRawValue();
        }

        virtual spring_value_t getValue() {
            if (_ready) {
                spring_value_t returnValue = _pin.getValue();
                bool devianceTooHigh = false;

                return returnValue;
            }

            return 0;
        }

        virtual void setCalibrationStatus(spring_calibration_t calibrationStatus) {
            if (calibrationStatus == SPRING_CURRENTLY_CALIBRATING) {
                _beginCalibration();
            } else if (calibrationStatus == SPRING_CURRENTLY_NOT_CALIBRATING) {
                _endCalibration();
            }
            _calibrationStatus = calibrationStatus;
        }

        virtual spring_calibration_t getCalibrationStatus() {
            return _calibrationStatus;
        }

        virtual void setStatus(status_t status) {
            // No implementation needed
        }
        virtual void setValue(spring_value_t value) {
            // No impemantation needed
        }

        virtual void receive(CarMessage &carMessage) {
            for (car_sub_message_t &subMessage : carMessage.subMessages) {
                switch (subMessage.data[0]) {
                    case SPRING_MESSAGE_COMMAND_SET_CALIBRATION_STATUS:
                        setCalibrationStatus((spring_calibration_t)subMessage.data[1]);
                        break;
                    
                    case SPRING_MESSAGE_COMMAND_SET_PROPORTIONALITY:
                        setProportionality((spring_sensor_type_t)subMessage.data[1]);
                        break;
                }
            }
        }

    protected:
        Ticker _springBothTicker;
        Ticker _statusTicker;

        HardwareAnalogSensor _pin;
        spring_sensor_type_t _proportionality = SPRING_DIRECT_PROPORTIONAL;

        status_t _status = 0;
        bool _ready = false;

        float _calibrationRefreshTime = STD_SPRING_CALIBRATION_REFRESH_TIME;
        uint16_t _calibrationMinDeviance = STD_SPRING_CALIBRATION_MIN_DEVIANCE;
        uint16_t _calibrationMaxDeviance = STD_SPRING_CALIBRATION_MAX_DEVIANCE;

        analog_sensor_raw_t _analogLowerBoundary = STD_SPRING_ANALOG_LOWER_BOUNDARY;
        analog_sensor_raw_t _analogUpperBoundary = STD_SPRING_ANALOG_UPPER_BOUNDARY;

        Ticker _calibrationTicker;
        spring_calibration_t _calibrationStatus = SPRING_CURRENTLY_NOT_CALIBRATING;

        struct _calibration {
            analog_sensor_raw_t min = 0, max = 0;
            bool initPointSet = false;
            CircularBuffer<analog_sensor_raw_t, STD_SPRING_CALIBRATION_SAMPLE_BUFFER_SIZE> buffer;
        } _calibration;

        void _updateBoth() {
            if (_syncerAttached) {
                uint16_t value = ((float)getValue() * 65535);
                uint16_t raw = getRaw();

                _sendCommand(SPRING_MESSAGE_COMMAND_SET_BOTH, value & 0xFF, (value >> 8) & 0xFF, raw & 0xFF, (raw >> 8) & 0xFF, SEND_PRIORITY_SUSPENSION, IS_DROPABLE);
            }
        }

        status_t _lastSentStatus = 0;
        void _updateStatus() {
            if (_syncerAttached) {
                status_t currentStatus = getStatus();
                if(_lastSentStatus != currentStatus) {
                    _lastSentStatus = currentStatus;
                    _sendCommand(SPRING_MESSAGE_COMMAND_SET_STATUS, currentStatus, SEND_PRIORITY_SUSPENSION, IS_NOT_DROPABLE);
                }
            }
        }

        analog_sensor_raw_t _getAverageValue(CircularBuffer<analog_sensor_raw_t, STD_SPRING_CALIBRATION_SAMPLE_BUFFER_SIZE> &buffer) {
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
            _calibration.buffer.push(_pin.getRawValue());
            if (_calibration.buffer.full()) {
                analog_sensor_raw_t currentValue = _getAverageValue(_calibration.buffer);

                if (_calibration.initPointSet) {
                    if (currentValue < _calibration.min)
                        _calibration.min = currentValue;
                    if (currentValue > _calibration.max)
                        _calibration.max = currentValue;
                } else {
                    _calibration.min = currentValue;
                    _calibration.max = currentValue;
                    _calibration.initPointSet = true;
                }
            }
        }

        virtual void _beginCalibration() {
            _calibration.initPointSet = false;
            _calibration.buffer.reset();

            _calibrationTicker.attach(callback(this, &HardwareSpringTravelSensor::_calibrationLoop), _calibrationRefreshTime);
        }

        virtual void _endCalibration() {
            _calibrationTicker.detach();
            bool devianceTooLow = false;
            bool devianceTooHigh = false;

            if (!_calibration.initPointSet) {
                devianceTooLow = true;

                _calibration.min = 0;
                _calibration.max = 0;
            }

            if ((_calibration.max - _calibration.min) >= _calibrationMinDeviance) {
                if ((_calibration.max - _calibration.min) <= _calibrationMaxDeviance) {
                    if (_proportionality == SPRING_DIRECT_PROPORTIONAL) {
                        _pin.setMapping(_calibration.min, _calibration.max, 0.0, 1.0);
                    } else if (_proportionality == SPRING_INDIRECT_PROPORTIONAL) {
                        _pin.setMapping(_calibration.max, _calibration.min, 0.0, 1.0);
                    } else {
                        spring_error_type_t calibrationFailedWrongConfigError = SPRING_CALIBRATION_FAILED_WRONG_CONFIG;
                        _status |= calibrationFailedWrongConfigError;
                        return;
                    }
                } else {
                    devianceTooHigh = true;
                }
            } else {
                devianceTooLow = true;
            }

            bool maybeReady = true;

            if (devianceTooLow) {
                spring_error_type_t calibrationFailedTooLowDevianceError = SPRING_CALIBRATION_FAILED_TOO_LOW_DEVIANCE;
                _status |= calibrationFailedTooLowDevianceError;
                maybeReady = false;
            }

            if (devianceTooHigh) {
                spring_error_type_t calibrationFailedTooHighDevianceError = SPRING_CALIBRATION_FAILED_TOO_HIGH_DEVIANCE;
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

#endif // HARDWARE_SPRING_TRAVEL_SENSOR_H