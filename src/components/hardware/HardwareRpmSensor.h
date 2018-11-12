#ifndef HARDWARERPMSENSOR_H
#define HARDWARERPMSENSOR_H

#include "../interface/IRpmSensor.h"

#define STD_MEASUREMENT_TIMEOUT 0.1 // s -> 100ms
#define STD_MEASUREMENT_POINTS_PER_REVOLUTION 8
//#define USE_FALL // STD it uses the rising edge. Decomment to use falling edge as measurement point

#define STD_AVERAGE_OVER_MEASUREMENT_POINTS 3 // Over how many measurement points should the real speed be calculated? HAS TO BE AT LEAST 1

class HardwareRpmSensor : public IRpmSensor {
    public:
        HardwareRpmSensor(PinName pin, uint8_t measurementPointsPerRevolution = STD_MEASUREMENT_POINTS_PER_REVOLUTION)
            : _pin(pin) {
            _measurement.pointsPerRevolution = measurementPointsPerRevolution;

            #ifdef USE_FALL
                _pin.fall(callback(this, &HardwareRpmSensor::_measurementEvent));
            #else
                _pin.rise(callback(this, &HardwareRpmSensor::_measurementEvent));
            #endif
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
                    returnValue += _measurement.buffer[i];
                }
                returnValue /= STD_AVERAGE_OVER_MEASUREMENT_POINTS;

                returnValue = 60000.0 / (returnValue * (rpm_sensor_frequency_t)_measurement.pointsPerRevolution); // 1/(ms / 60000) == 60000/ms -> rpm
            }

            return returnValue;
        }

    protected:
        InterruptIn _pin;

        struct _measurement {
            Timer timer;
            Ticker timeOut;
            uint8_t pointsPerRevolution;
            bool started = false;
            bool zero = true;

            uint16_t buffer[STD_AVERAGE_OVER_MEASUREMENT_POINTS];
            uint8_t bufferSize = 0;
        } _measurement;

        void _measurementEvent() {
            _measurement.timeOut.detach();

            if (_measurement.started) {
                uint16_t currentTime = _measurement.timer.read_ms(); // ms since last edge
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