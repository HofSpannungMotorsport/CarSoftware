#ifndef SPEEDSERVICE_H
#define SPEEDSERVICE_H

#include "IService.h"
#include "SCar.h"
#include "../interface/IRpmSensor.h"
#include "../interface/IMotorController.h"

#define STD_SPEED_DEVIANCE_THRESHHOLD 3 // kM/h -> if one sensor gives a higher Value than this, the other one will be compared
#define STD_MAX_SPEED_DEVIANCE 0.1 // 10%

#define STD_DISTANCE_PER_REVOLUTION 1.4451326206513048896928159563086 // m -> The distance a wheel will travel over one whole rotation
#define STD_MOTOR_TO_WHEEL_RATIO (1.0/3.6) // The gear Ratio from the Motor to the rear Wheels

// New Values to Calculate the Power for the Adaptive Power Control
#define THROTTLE_ACTIVE
#define ACCU_MIN_VOLTAGE 330 // V =~ Voltage at full power and about half full accu
#define ACCU_MAX_VOLTAGE 400 // V = Completely full Accu (will never happen)
#define ACCU_MAX_ALLOWED_CURRENT 200 // A
#define MOTOR_MAX_VOLTAGE 470 // V = Max Motor Voltage (according to datasheet)
#define MOTOR_MAX_VOLTAGE_SPEED_UNDER_LOAD 5170 // RPM
#define INVERTER_MAX_ALLOWED_CURRENT 381.8 // A (under field I max pk)

// Gets calculated at compilation
#define ACCU_POWER_MAX_UNDER_LOAD (ACCU_MIN_VOLTAGE * ACCU_MAX_ALLOWED_CURRENT) // Max Power under Load
#define MOTOR_LOWER_BOUNDARY_VOLTAGE (ACCU_POWER_MAX_UNDER_LOAD / INVERTER_MAX_ALLOWED_CURRENT) // Voltage where the current gets adaptively reduced
#define MOTOR_RPM_TO_KMH (STD_DISTANCE_PER_REVOLUTION * STD_MOTOR_TO_WHEEL_RATIO * 0.06) // =~ 0.02408554367752174816154693260514
#define MOTOR_VOLTAGE_TO_RPM_MULTIPLYER (MOTOR_MAX_VOLTAGE_SPEED_UNDER_LOAD / MOTOR_MAX_VOLTAGE)
#define INVERTER_MAX_POSSIBLE_POWER (ACCU_MAX_VOLTAGE * INVERTER_MAX_ALLOWED_CURRENT) // only for Info


// Calculated Values for the adaptive power control
#define THROTTLE_FROM (MOTOR_LOWER_BOUNDARY_VOLTAGE * MOTOR_VOLTAGE_TO_RPM_MULTIPLYER * MOTOR_RPM_TO_KMH) // only for Info

/*
    Speed is measured by the front Wheels -> most accurat result.
    If the Sensors in the front have a problem, the ones in the back will be used instead.
    If the Sensors in the back have a problem too, the Speed of the Motor will be used.
*/

enum speed_service_error_types_t : uint8_t {
    SPEED_SERVICE_NO_SENSOR_WORKING = 0x1,
    SPEED_SERVICE_USING_MOTOR =       0x2
};

typedef float speed_value_t;
class SSpeed : public IService {
    public:
        SSpeed(SCar &carService,
                     IRpmSensor* rpmFrontLeft, IRpmSensor* rpmFrontRight, /* IRpmSensor* rpmRearLeft, IRpmSensor* rpmRearRight, */
                     IMotorController* motorController)
            : _carService(carService) {
            _rpm.front.left = rpmFrontLeft;
            _rpm.front.right = rpmFrontRight;
            //_rpm.rear.left = rpmRearLeft;
            //_rpm.rear.right = rpmRearRight;
            _motorController = motorController;
        }

        virtual void run() {
            enum useSensor : uint8_t {
                FRONT,
                REAR,
                MOTOR
            } useSensor;

            if ((_rpm.front.left->getStatus() > 0) || (_rpm.front.right->getStatus() > 0)) {
                // One of the front Sensors has a problem
                //if ((_rpm.rear.left->getStatus() > 0) || (_rpm.rear.right->getStatus() > 0)) {
                    // One of the rear Sensors has a problem too
                    if (_motorController->getStatus() > 0) {
                        _speed = 0;
                        _carService.addError(Error(componentId::getComponentId(COMPONENT_SYSTEM, COMPONENT_SYSTEM_SPEED), SPEED_SERVICE_NO_SENSOR_WORKING, ERROR_ISSUE));
                        return;
                    } else {
                        useSensor = MOTOR;
                    }
                //} else {
                //    useSensor = REAR;
                //}
            } else {
                useSensor = FRONT;
            }

            #ifdef SSPEED_FORCED_USE_MOTOR
                useSensor = MOTOR;
            #endif

            if (useSensor == FRONT) {
                if (/*_checkPlausibility(_rpm.front.left, _rpm.front.right)*/ true) {
                    // Calculate mid. of both sensors, then rpm -> m/min -> km/h
                    _speed = _getSpeed(_rpm.front.left, _rpm.front.right);
                } else {
                    //if ((_rpm.rear.left->getStatus() > 0) || (_rpm.rear.right->getStatus() > 0)) {
                        useSensor = MOTOR;
                    //} else {
                    //    useSensor = REAR;
                    //}
                }
            }

            /*
            if (useSensor == REAR) {
                if (_checkPlausibility(_rpm.rear.left, _rpm.rear.right)) {
                    _speed = _getSpeed(_rpm.rear.left, _rpm.rear.right);
                } else {
                    useSensor = MOTOR;
                }
            }
            */

            if (useSensor == MOTOR) {
                _speed = _getSpeed(_motorController);
                //_carService.addError(Error(ID::getComponentId(SYSTEM, SYSTEM_SPEED), SPEED_SERVICE_USING_MOTOR, ERROR_ISSUE));
            }

            #ifdef SSPEED_REPORT_SPEED
                pcSerial.printf("[SSpeed]@run: Current Speed: %.3f kM/h\n", _speed);
            #endif

            #ifndef SSPEED_DISABLE_CURRENT_LIMITATION
                _setThrottle(_speed);
            #endif
        }

        speed_value_t getSpeed() {
            return _speed;
        }

    protected:
        SCar &_carService;
        IMotorController* _motorController;

        speed_value_t _speed;
        
        struct _rpm {
            struct front {
                IRpmSensor* left;
                IRpmSensor* right;
            } front;

            /*
            struct rear {
                IRpmSensor* left;
                IRpmSensor* right;
            } rear;
            */
        } _rpm;

        speed_value_t _getSpeed(IRpmSensor* sensor) {
            return (sensor->getFrequency() * (speed_value_t)STD_DISTANCE_PER_REVOLUTION * (speed_value_t)0.06);
        }

        speed_value_t _getSpeed(IRpmSensor* sensor1, IRpmSensor* sensor2) {
            return ((sensor1->getFrequency() + sensor2->getFrequency()) / 2.0) * (speed_value_t)STD_DISTANCE_PER_REVOLUTION * (speed_value_t)0.06;
        }

        speed_value_t _getSpeed(IMotorController* sensor) {
            float motorControllerRpm = sensor->getSpeed();

            #ifdef SSPEED_REPORT_MOTOR_RPM
                pcSerial.printf("[SSpeed]@run: Current Motor RPM: %.3f RPM\n", motorControllerRpm);
            #endif

            return (motorControllerRpm * STD_MOTOR_TO_WHEEL_RATIO * STD_DISTANCE_PER_REVOLUTION * 0.06);
        }

        bool _checkPlausibility(IRpmSensor* sensor1, IRpmSensor* sensor2) {
            speed_value_t sensor1speed = _getSpeed(sensor1),
                          sensor2speed = _getSpeed(sensor2);
            
            if ((sensor1speed > STD_SPEED_DEVIANCE_THRESHHOLD) || (sensor2speed > STD_SPEED_DEVIANCE_THRESHHOLD)) {
                float deviance = 0;

                if (sensor1speed > sensor2speed) {
                    deviance = 1 - (sensor2speed / sensor1speed);
                } else if (sensor2speed > sensor1speed) {
                    deviance = 1 - (sensor1speed / sensor2speed);
                }

                if (deviance > STD_MAX_SPEED_DEVIANCE) {
                    return false;
                }
            }

            return true;
        }

        float _map(float x, float in_min, float in_max, float out_min, float out_max) {
            return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
        }

        void _setThrottle(speed_value_t speed) {
            #ifdef THROTTLE_ACTIVE

            float currentVoltage = ((speed / ((float)MOTOR_MAX_VOLTAGE_SPEED_UNDER_LOAD * (float)MOTOR_RPM_TO_KMH)) * MOTOR_MAX_VOLTAGE);
            if (currentVoltage > (float)ACCU_MAX_VOLTAGE) currentVoltage = (float)ACCU_MAX_VOLTAGE;

            float currentMaxPossiblePower = currentVoltage * (float)INVERTER_MAX_ALLOWED_CURRENT;
            float currentPowerLimit = (float)ACCU_POWER_MAX_UNDER_LOAD / currentMaxPossiblePower;

            if (currentPowerLimit > 1.0) currentPowerLimit = 1.0;
            if (currentPowerLimit < 0.0) currentPowerLimit = 0.0;

            _carService.setMaxPower(currentPowerLimit);

            #endif
        }
};

#endif // SPEEDSERVICE_H