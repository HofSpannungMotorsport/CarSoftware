#ifndef SPEEDSERVICE_H
#define SPEEDSERVICE_H

#include "IService.h"
#include "SCar.h"
#include "../interface/IRpmSensor.h"
#include "../interface/IMotorController.h"

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
                     /*IRpmSensor &rpmFrontLeft, IRpmSensor &rpmFrontRight,  IRpmSensor &rpmRearLeft, IRpmSensor &rpmRearRight, */
                     IMotorController &motorController, IRegistry &registry)
            : _carService(carService),
            /*_rpm.front.left(rpmFrontLeft), _rpm.front.right(rpmFrontRight), _rpm.rear.left(rpmRearLeft), _rpm.rear.right(rpmRearRight), */
              _motorController(motorController), _registry(registry) {}

        virtual void run() {
            enum useSensor : uint8_t {
                FRONT,
                REAR,
                MOTOR
            } useSensor;

            //if ((_rpm.front.left.getStatus() > 0) || (_rpm.front.right.getStatus() > 0)) {
                // One of the front Sensors has a problem
                //if ((_rpm.front.left.getStatus() > 0) || (_rpm.front.right.getStatus() > 0)) {
                    // One of the rear Sensors has a problem too
                    if (_motorController.getStatus() > 0) {
                        _speed = 0;
                        _carService.addError(Error(componentId::getComponentId(COMPONENT_SYSTEM, COMPONENT_SYSTEM_SPEED), SPEED_SERVICE_NO_SENSOR_WORKING, ERROR_ISSUE));
                        return;
                    } else {
                        useSensor = MOTOR;
                    }
                //} else {
                //    useSensor = REAR;
                //}
            //} else {
                //useSensor = FRONT;
            //}

            /*
            if (useSensor == FRONT) {
                if (_checkPlausibility(_rpm.front.left, _rpm.front.right)) {
                    // Calculate mid. of both sensors, then rpm -> m/min -> km/h
                    _speed = (_getSpeed(_rpm.front.left) + _getSpeed(_rpm.front.right)) / 2;
                } else {
                    if ((_rpm.front.left.getStatus() > 0) || (_rpm.front.right.getStatus() > 0)) {
                        useSensor = MOTOR;
                    } else {
                        useSensor = REAR;
                    }
                }
            }
            */

            /*
            if (useSensor == REAR) {
                if (_checkPlausibility(_rpm.rear.left, _rpm.rear.right)) {
                    _speed = (_getSpeed(_rpm.rear.left) + _getSpeed(_rpm.rear.right)) / 2;
                } else {
                    useSensor = MOTOR;
                }
            }
            */

            if (useSensor == MOTOR) {
                _speed = _getSpeed(_motorController);
                //_carService.addError(Error(ID::getComponentId(SYSTEM, SYSTEM_SPEED), SPEED_SERVICE_USING_MOTOR, ERROR_ISSUE));
            }
        }

        speed_value_t getSpeed() {
            return _speed;
        }

    protected:
        IRegistry &_registry;

        SCar &_carService;
        IMotorController &_motorController;

        speed_value_t _speed;
        
        struct _rpm {
            /*
            struct front {
                IRpmSensor &left;
                IRpmSensor &right;
            } front;

            struct rear {
                IRpmSensor &left;
                IRpmSensor &right;
            } rear;
            */
        } _rpm;

        speed_value_t _getSpeed(IRpmSensor &sensor) {
            return (sensor.getFrequency() * _registry.getFloat(SSPEED_DISTANCE_PER_REVOLUTION) * 0.06);
        }

        speed_value_t _getSpeed(IMotorController &sensor) {
            return (sensor.getSpeed() * _registry.getFloat(SSPEED_MOTOR_TO_WHEEL_RATIO) * _registry.getFloat(SSPEED_DISTANCE_PER_REVOLUTION) * 0.06);
        }

        bool _checkPlausibility(IRpmSensor &sensor1, IRpmSensor &sensor2) {
            speed_value_t sensor1speed = _getSpeed(sensor1),
                          sensor2speed = _getSpeed(sensor2);
            
            float speedDevianceThreshhold = _registry.getFloat(SSPEED_SPEED_DEVIANCE_THRESHHOLD);
            if ((sensor1speed > speedDevianceThreshhold) || (sensor2speed > speedDevianceThreshhold)) {
                float deviance = 0;

                if (sensor1speed > sensor2speed) {
                    deviance = 1 - (sensor2speed / sensor1speed);
                } else if (sensor2speed > sensor1speed) {
                    deviance = 1 - (sensor1speed / sensor2speed);
                }

                if (deviance > _registry.getFloat(SSPEED_MAX_SPEED_DEVIANCE)) {
                    return false;
                }
            }

            return true;
        }
};

#endif // SPEEDSERVICE_H