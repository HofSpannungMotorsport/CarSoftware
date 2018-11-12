#ifndef IRPMSENSOR_H
#define IRPMSENSOR_H

typedef uint8_t rpm_sensor_status_t;
struct rpm_sensor_error_type_t {
    
};

typedef float rpm_sensor_speed_t; // m/s
typedef float rpm_sensor_frequency_t; // rpm

class IRpmSensor : public IID {
    public:
        virtual void setMeasurementPointsPerRevolution(uint8_t measurementPointsPerRevolution) = 0;
        virtual void setFrequency(rpm_sensor_frequency_t frequency) = 0;

        virtual uint8_t getMeasurementsPerRevolution() = 0;
        virtual rpm_sensor_frequency_t getFrequency() = 0;
};

#endif // IRPMSENSOR_H