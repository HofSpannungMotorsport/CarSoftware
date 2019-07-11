#ifndef HARDWAREPEDAL_H
#define HARDWAREPEDAL_H

#include "platform/CircularBuffer.h"
#include "../interface/IPedal.h"
#include "../hardware/HardwareAnalogSensor.h"

// Hardcoded Calibration. Can be overwritten by recalibration if any fault
#define STD_GAS_1_MIN 18400
#define STD_GAS_1_MAX 45800

#define STD_GAS_2_MIN 13450
#define STD_GAS_2_MAX 43500

#define STD_BRAKE_MIN 13470
#define STD_BRAKE_MAX 19200

// Devinance Settings
#define STD_MAX_DEVIANCE 0.35 // 10%
#define STD_MAX_DEVIANCE_TIME 200 // 100ms

#define STD_MAPPED_BOUNDARY_PERCENTAGE 0.25 // 10%
#define STD_MAX_OUT_OF_BOUNDARY_TIME 200 // 100ms

#define STD_CALIBRATION_REFRESH_TIME        0.030 // 30ms
#define STD_CALIBRATION_MIN_DEVIANCE        500 // Raw analog
#define STD_CALIBRATION_MAX_DEVIANCE        50000 // Raw analog
#define STD_CALIBRATION_SAMPLE_BUFFER_SIZE  20 // How many values should be combined during calibration to get the fu***** deviance away

#define STD_ANALOG_LOWER_BOUNDARY   655 // uint16_t min ->     0
#define STD_ANALOG_UPPER_BOUNDARY 64880 // uint16_t max -> 65535

#define STD_PEDAL_THRESHHOLD 0.15 // 15%

struct pedal_calibration_data_t {
    uint16_t min[2] = {0,0}, max[2] = {0,0};
    bool secondSensor = false;
};

class HardwarePedal : public IPedal {
    public:
        HardwarePedal(PinName inputPin)
            : _pin1(inputPin), _pin2(inputPin){
            _secondSensor = false;
            _init();
        }

        HardwarePedal(PinName inputPin, id_sub_component_t componentSubId)
            : HardwarePedal(inputPin) {
            setComponentSubId(componentSubId);
        }

        HardwarePedal(PinName inputPin, id_sub_component_t componentSubId, uint16_t min, uint16_t max)
            : HardwarePedal(inputPin, componentSubId) {
            _pin1.setMapping(min, max, 0.0, 1.0);
            _calibrationSet = true;
            _ready = true;
        }

        HardwarePedal(PinName inputPin1, PinName inputPin2)
            : _pin1(inputPin1), _pin2(inputPin2) {
            _secondSensor = true;
            _init();
        }

        HardwarePedal(PinName inputPin1, PinName inputPin2, id_sub_component_t componentSubId)
            : HardwarePedal(inputPin1, inputPin2) {
            setComponentSubId(componentSubId);
        }

        HardwarePedal(PinName inputPin1, PinName inputPin2, id_sub_component_t componentSubId, uint16_t minPin1, uint16_t maxPin1, uint16_t minPin2, uint16_t maxPin2)
            : HardwarePedal(inputPin1, inputPin2, componentSubId) {
            _pin1.setMapping(minPin1, maxPin1, 0.0, 1.0);
            _pin2.setMapping(minPin2, maxPin2, 0.0, 1.0);
            _calibrationSet = true;
            _ready = true;
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

        virtual bool getCalibration(pedal_calibration_data_t &pedalCalibration) {
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

        virtual message_build_result_t buildMessage(CarMessage &carMessage) {
            car_sub_message_t subMessage;

            subMessage.length = 3;

            subMessage.data[0] = this->getStatus();

            // change line below when type of pedal_value_t changes
            float pedalValueFloat = this->getValue();
            uint16_t pedalValue = ((float)pedalValueFloat * 65535);

            #ifdef PEDAL_MESSAGE_HANDLER_DEBUG
                pcSerial.printf("[HardwarePedal]@buildMessage: HardwareObject (float)pedalValue: %.3f\t(uint16_t)pedalValue: %i\t", pedalValueFloat, pedalValue);
            #endif

            subMessage.data[1] = pedalValue & 0xFF;
            subMessage.data[2] = (pedalValue >> 8) & 0xFF;

            #ifdef PEDAL_MESSAGE_HANDLER_DEBUG
                pcSerial.printf("msg.data[1]: 0x%x\tmsg.data[2]: 0x%x\n", subMessage.data[1], subMessage.data[2]);
            #endif

            carMessage.addSubMessage(subMessage);

            return MESSAGE_BUILD_OK;
        }

        virtual message_parse_result_t parseMessage(CarMessage &carMessage) {
            message_parse_result_t result = MESSAGE_PARSE_OK;
            for (car_sub_message_t &subMessage : carMessage.subMessages) {
                if(subMessage.length != 1) // not a valid message
                    result = MESSAGE_PARSE_ERROR;

                uint8_t gotValue = subMessage.data[0];
                pedal_calibration_t calibrationStatus = CURRENTLY_NOT_CALIBRATING;

                if (gotValue == 1) {
                    calibrationStatus = CURRENTLY_NOT_CALIBRATING;
                } else if (gotValue == 2) {
                    calibrationStatus = CURRENTLY_CALIBRATING;
                }

                this->setCalibrationStatus(calibrationStatus);

                #ifdef PEDAL_MESSAGE_HANDLER_DEBUG
                    pcSerial.printf("[HardwarePedal]@parseMessage: SoftwareObject calibrationStatus: 0x%x\tmsg.data[0]: 0x%x\tgotValue: %i\n", calibrationStatus, subMessage.data[0], gotValue);
                #endif
            }
            
            return result;
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
        bool _calibrationSet = false;

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
            _ready = false;

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