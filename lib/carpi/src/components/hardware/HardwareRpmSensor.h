#ifndef HARDWARERPMSENSOR_H
#define HARDWARERPMSENSOR_H

#include "../interface/IRpmSensor.h"

#define STD_MEASUREMENT_TIMEOUT 0.5 // s -> 500ms
#define STD_MEASUREMENT_POINTS_PER_REVOLUTION 12
//#define USE_FALL // STD it uses the rising edge. Decomment to use falling edge as measurement point

#define STD_AVERAGE_OVER_MEASUREMENT_POINTS 2 // Over how many measurement points should the real speed be calculated? HAS TO BE AT LEAST 1

class HardwareRpmSensor : public IRpmSensor {
    public:
        HardwareRpmSensor(PinName pin)
            : _pin(pin) {
            #ifdef USE_FALL
                _pin.fall(callback(this, &HardwareRpmSensor::_measurementEvent));
            #else
                _pin.rise(callback(this, &HardwareRpmSensor::_measurementEvent));
            #endif

            setComponentType(COMPONENT_RPM_SENSOR);
            setObjectType(OBJECT_HARDWARE);
        }

        HardwareRpmSensor(PinName pin, id_sub_component_t componentSubId, uint8_t measurementPointsPerRevolution = STD_MEASUREMENT_POINTS_PER_REVOLUTION)
            : HardwareRpmSensor(pin) {
            setComponentSubId(componentSubId);
            _measurement.pointsPerRevolution = measurementPointsPerRevolution;
        }

        virtual void setStatus(rpm_sensor_status_t status) {
            // No implementation needed
        }

        virtual rpm_sensor_status_t getStatus() {
            return _status;
        }

        virtual void setMeasurementPointsPerRevolution(uint8_t measurementPointsPerRevolution) {
            _measurement.pointsPerRevolution = measurementPointsPerRevolution;
        }

        virtual void setFrequency(rpm_sensor_frequency_t frequency) {
            // No implementation needed
        }

        virtual uint8_t getMeasurementsPerRevolution() {
            return _measurement.pointsPerRevolution;
        }

        virtual rpm_sensor_frequency_t getFrequency() {
            rpm_sensor_frequency_t returnValue = 0;
            if (!_measurement.zero) {
                for (uint8_t i = 0; i < STD_AVERAGE_OVER_MEASUREMENT_POINTS; i++) {
                    returnValue += ((rpm_sensor_frequency_t)_measurement.buffer[i]) / 1000; // -> ms between measurement points
                }
                returnValue /= (rpm_sensor_frequency_t)STD_AVERAGE_OVER_MEASUREMENT_POINTS; // -> ms per revolution

                returnValue = 60000 / (returnValue * (rpm_sensor_frequency_t)_measurement.pointsPerRevolution); // 1/(ms / 60000) == 60000/ms -> rpm
            }

            return returnValue;
        }

        virtual message_build_result_t buildMessage(CarMessage &carMessage) {
            car_sub_message_t subMessage;
            subMessage.length = 5;

            subMessage.data[0] = this->getStatus();

            rpm_sensor_frequency_t frequency = this->getFrequency();
            uint32_t frequencyBinary = *((uint32_t*)&frequency);

            // Slice data in 4 Byte -> 32bit float
            for (uint8_t i = 1; i < 5; i++) {
                subMessage.data[i] = (uint8_t)((frequencyBinary >> ((i - 1) * 8)) & 0xFF);
            }

            carMessage.addSubMessage(subMessage);

            return MESSAGE_BUILD_OK;
        }

        virtual message_parse_result_t parseMessage(CarMessage &carMessage) {
            // No implementation needed
            return MESSAGE_PARSE_ERROR;
        }

        float getAge() {
            return 0;
        }

    protected:
        InterruptIn _pin;
        rpm_sensor_status_t _status = 0;

        struct _measurement {
            Timer timer;
            Ticker timeOut;
            uint8_t pointsPerRevolution;
            bool started = false;
            bool zero = true;

            us_timestamp_t buffer[STD_AVERAGE_OVER_MEASUREMENT_POINTS];
            uint8_t bufferSize = 0;
        } _measurement;

        void _measurementEvent() {
            _measurement.timeOut.detach();

            if (_measurement.started) {
                us_timestamp_t currentTime = _measurement.timer.read_high_resolution_us(); // ms since last edge
                _measurement.timer.reset();

                // Shift buffer
                for (uint8_t i = (STD_AVERAGE_OVER_MEASUREMENT_POINTS-1); i > 0; i--) {
                    _measurement.buffer[i] = _measurement.buffer[i-1];
                }
                // Put new time at the beginning of the buffer
                _measurement.buffer[0] = currentTime;

                if (_measurement.zero) {
                    if (_measurement.bufferSize < STD_AVERAGE_OVER_MEASUREMENT_POINTS) {
                        _measurement.bufferSize++;
                    }

                    if (_measurement.bufferSize == STD_AVERAGE_OVER_MEASUREMENT_POINTS) {
                        _measurement.zero = false;
                    }
                }
            } else {
                _measurement.timer.reset();
                _measurement.timer.start();
                _measurement.started = true;
            }

            _measurement.timeOut.attach(callback(this, &HardwareRpmSensor::_measurementTimeOut), STD_MEASUREMENT_TIMEOUT);
        }

        void _measurementTimeOut() {
            _measurement.timeOut.detach();
            _measurement.timer.stop();
            _measurement.zero = true;
            _measurement.started = false;
            _measurement.bufferSize = 0;
        }
};

#endif // HARDWARERPMSENSOR_H