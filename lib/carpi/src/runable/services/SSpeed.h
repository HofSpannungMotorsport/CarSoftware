#ifndef SPEEDSERVICE_H
#define SPEEDSERVICE_H

#include "IService.h"
#include "SCar.h"
#include "../interface/IRpmSensor.h"
#include "../interface/IMotorController.h"

#define STD_SPEED_DEVIANCE_THRESHHOLD 3 // kM/h -> if one sensor gives a higher Value than this, the other one will be compared
#define STD_MAX_SPEED_DEVIANCE 0.1 // 10%

#define STD_DISTANCE_PER_REVOLUTION 1.43633616122 // m -> The distance a wheel will travel over one whole rotation
#define STD_MOTOR_TO_WHEEL_RATIO (1.0/3.6) // The gear Ratio from the Motor to the rear Wheels

// Gets calculated at compiletime
#define MOTOR_RPM_TO_KMH (STD_DISTANCE_PER_REVOLUTION * STD_MOTOR_TO_WHEEL_RATIO * 0.06) // =~ 0.02408554367752174816154693260514

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
                     IRpmSensor* rpmFrontLeft, IRpmSensor* rpmFrontRight,
                     IMotorController* motorController)
            : _carService(carService) {
            _rpm.front.left = rpmFrontLeft;
            _rpm.front.right = rpmFrontRight;
            _motorController = motorController;
        }

        virtual void run() {
            enum useSensor : uint8_t {
                FRONT,
                REAR,
                MOTOR
            } useSensor;

            if ((_rpm.front.left->getStatus() > 0) || (_rpm.front.right->getStatus() > 0)) {
                if (_motorController->getStatus() > 0) {
                    _speed = 0;
                    _carService.addError(Error(componentId::getComponentId(COMPONENT_SYSTEM, COMPONENT_SYSTEM_SPEED), SPEED_SERVICE_NO_SENSOR_WORKING, ERROR_ISSUE));
                    return;
                } else {
                    useSensor = MOTOR;
                }
            } else {
                useSensor = FRONT;
            }

            #ifdef SSPEED_FORCED_USE_MOTOR
                useSensor = MOTOR;
            #endif

            if (useSensor == FRONT) {
                _speed = _getSpeed(_rpm.front.left, _rpm.front.right);
            }

            if (useSensor == MOTOR) {
                _speed = _getSpeed(_motorController);
            }

            #ifdef SSPEED_REPORT_SPEED
                pcSerial.printf("[SSpeed]@run: Current Speed: %.3f kM/h\n", _speed);
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

            return (motorControllerRpm * MOTOR_RPM_TO_KMH);
        }
};

#endif // SPEEDSERVICE_H