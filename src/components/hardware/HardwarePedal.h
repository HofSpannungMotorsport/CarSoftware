#ifndef HARDWAREPEDAL_H
#define HARDWAREPEDAL_H

#include "mbed.h"
#include "../interface/IPedal.h"
#include "../interface/IAnalogSensor.h"

class HardwarePedal : public IPedal {
    public:

        HardwarePedal(uint8_t count, IAnalogSensor *sensor) {
            _sensorCount = count;
            _sensors = sensor;
        }

        virtual ~HardwarePedal() {
            
        }

        virtual execution_result_t execute()
        {
            // TODO read values
        }

        virtual pedal_status_t getStatus() {
            return _status;
        }

        virtual pedal_value_t getValue() {
            return getSensorValue();
        }
        
        virtual void setStatus(pedal_status_t status) {
            // no implementation needed
        }
        
        virtual void setValue(pedal_value_t value) {
            // no implementation needed
        }

    private:
        IAnalogSensor *_sensors;
        uint8_t _sensorCount;
        pedal_status_t _status;

        analog_sensor_normval_t getSensorValue() {
            _status = 0;
            //determine sensors
            if(_sensorCount==1) {
                analog_sensor_normval_t val = _sensors[0].getNormalizedValue();
                if(_sensors[0].getStatus() != 0) {
                    _status |= 1<<PEDAL_SENSOR_PROBLEM;
                    return 0;
                }

                return val;
            }

            if(_sensorCount == 2) {
                analog_sensor_normval_t val1 = _sensors[0].getNormalizedValue();
                analog_sensor_normval_t val2 = _sensors[1].getNormalizedValue(); 

                if((_sensors[0].getStatus() != 0) || (_sensors[1].getStatus() != 0)) {
                    _status |= 1<<PEDAL_SENSOR_PROBLEM;
                    return 0;
                }

                analog_sensor_normval_t deviance;
                // get abs, dont know a function..
                if(val1>val2)
                    deviance = val1-val2;
                else
                    deviance = val2-val1;

                if(deviance > 0.1) {
                    _status |= 1<< PEDAL_DEVIANCE_TOO_HIGH;
                    return 0;
                }

                return (val1+val2)/2;
            }
        }
};

#endif