#ifndef SPEEDSERVICE_H
#define SPEEDSERVICE_H

#include "IService.h"

#define STD_SPEED_DEVIANCE_THRESHHOLD 3 // kM/h -> if one sensor gives a higher Value than this, the other one will be compared
#define STD_MAX_SPEED_DEVIANCE 0.1 // 10%

#define STD_DISTANCE_PER_REVOLUTION 1.4363361612212534686251205548354 // m -> The distance a wheel will travel over one whole rotation

/*
    Speed is measured by the front Wheels -> most accurat result.
    If the Sensors in the front have a problem, the ones in the back will be used instead.
    If the Sensors in the back have a problem too, the Speed of the Motor will be used.
*/

typedef float speed_value_t;

class SpeedService : public IService {
    public:
        SpeedService(CarService &carService,
                     IRpmSensor* rpmFrontLeft, IRpmSensor* rpmFrontRight, IRpmSensor* rpmRearLeft, IRpmSensor* rpmRearRight
                     IMotorController* motorController)
            : _carService(carService) {
            _rpm.front.left = rpmFrontLeft;
            _rpm.front.right = rpmFrontRight;
            _rpm.rear.left = rpmRearLeft;
            _rpm.rear.right = rpmRearRight;
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
                if ((_rpm.front.left->getStatus() > 0) || (_rpm.front.right->getStatus() > 0)) {
                    // One of the rear Sensors has a problem too
                    if (_motorController->getStatus() > 0) {
                        _speed = 0;
                        _carService.addError(Error(ID::getComponentId(SYSTEM, SYSTEM_SPEED), 0x0, ERROR_ISSUE));
                        return;
                    } else {
                        useSensor = MOTOR;
                    }
                } else {
                    useSensor = REAR;
                }
            } else {
                useSensor = FRONT;
            }

            if (useSensor == FRONT) {
                if (_checkPlausibility(_rpm.front.left, _rpm.front.right)) {
                    // Calculate mid. of both sensors, then rpm -> m/min -> km/h
                    _speed = (_getSpeed(_rpm.front.left) + _getSpeed(_rpm.front.right)) / 2;
                } else {
                    if ((_rpm.front.left->getStatus() > 0) || (_rpm.front.right->getStatus() > 0)) {
                        useSensor = MOTOR;
                    } else {
                        useSensor = REAR;
                    }
                }
            }

            if (useSensor == REAR) {
                if (_checkPlausibility(_rpm.rear.left, _rpm.rear.right)) {
                    _speed = (_getSpeed(_rpm.rear.left) + _getSpeed(_rpm.rear.right)) / 2;
                } else {
                    useSensor = MOTOR;
                }
            }

            if (useSensor == MOTOR) {
                _speed = 0;
                _carService.addError(Error(ID::getComponentId(SYSTEM, SYSTEM_SPEED), 0x0, ERROR_ISSUE));
            }
        }

        speed_value_t getSpeed() {
            return _speed;
        }

    protected:
        CarService &_carService;
        IMotorController* _motorController;

        speed_value_t _speed;
        
        struct _rpm {
            struct front {
                IRpmSensor* left;
                IRpmSensor* right;
            } front;

            struct rear {
                IRpmSensor* left;
                IRpmSensor* right;
            } rear;
        } _rpm;

        speed_value_t _getSpeed(IRpmSensor* sensor) {
            return (sensor->getFrequency() * STD_DISTANCE_PER_REVOLUTION * 0.06);
        }

        void _checkPlausibility(IRpmSensor* sensor1, IRpmSensor* sensor2) {
            speed_value_t sensor1speed = _getSpeed(sensor1);
                          sensor2speed = _getSpeed(sensor2);
            
            if ((sensor1speed > STD_SPEED_DEVIANCE_THRESHHOLD) || (sensor2speed > STD_SPEED_DEVIANCE_THRESHHOLD)) {
                if (((sensor1speed / sensor2speed) > STD_MAX_SPEED_DEVIANCE)  ||  ((sensor2speed / sensor1speed) > STD_MAX_SPEED_DEVIANCE)) {
                    return false;
                }
            }

            return true;
        }
};

#endif // SPEEDSERVICE_H