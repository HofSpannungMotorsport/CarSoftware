#ifndef MOTORCONTROLLERSERVICE_H
#define MOTORCONTROLLERSERVICE_H

#include "mbed.h"
#include "IService.h"
#include "CarService.h"
#include <memory>
#include "../../can/can_ids.h"

#include "../interface/IMotorController.h"
#include "../interface/IPedal.h"
#include "../interface/IRpmSensor.h"

#define STD_MAX_POWER 80 // kW
#define STD_POWER_SET_ON_MOTOR_CONTROLLER 80 // kW
#define STD_AGE_LIMIT 0.1 // s

#define STD_BRAKE_POWER_LOCK_THRESHHOLD 0.02 // 2% -> if brake is put down only this amount, the Gas Pedal will be blocked

class MotorControllerService : public IService {
    public:
        MotorControllerService(CarService &carService,
                               IMotorController* motorController,
                               IPedal* gasPedal, IPedal* brakePedal) {
            _carService = carService;
            _setBasicComponents(motorController, gasPedal, brakePedal);
        }

        MotorCOntrollerService(CarService &carService,
                               IMotorController* motorController,
                               IPedal* gasPedal, IPedal* brakePedal,
                               IRpmSensor* frontLeftWheel, IRpmSensor* frontRightWheel, IRpmSensor* rearLeftWheel, IRpmSensor* rearRightWheel) {
            _carService = carService;
            _setBasicComponents(motorController, gasPedal, brakePedal);
            _setASRComponents(frontLeftWheel, frontRightWheel, rearLeftWheel, rearRightWheel);
        }

        virtual void run() {
            // At first, get all the new Values from the Components
            _updateValues();

            // Then, check for errors and register all of them in the CarService.
            // Also check for outdated Values (maybe because of CAN Disconnection)
            // and register it as error too.
            _checkErrors();

            float returnValue = 0;
            // Only if ready, set calculate Power
            if (_ready) {
                // Get pedal status (if brake is pushed -> gas pedal will be locked -> returns 0)
                float returnValue = (float)_getPedalPower();

                if (_asrActive) {
                    returnValue = _ASR(returnValue);
                }

                // Map the Value to match the power limit/set power
                returnValue = _mapToPowerLimit(returnValue);
            }

            // Send new Power to Motor -> Brum Brum (but without the Brum Brum)
            // ...maybe a drivers scream ;)
            _motorController->setTorque(returnValue);
        }

    protected:
        CarService &_carService;
        IMotorController* _motorController;
        bool _ready = false;

        struct _power {
            float max = STD_MAX_POWER,
                  setOnController = STD_POWER_SET_ON_MOTOR_CONTROLLER;
        } _power;

        struct _pedalStruct_t {
            IPedal* object;
            pedal_value_t lastValue;
            Timer age;
            bool ageStarted;
        };

        _pedalStruct_t _gasPedal,
                       _brakePedal;

        struct _rpmSensorStruct_t {
            IRpmSensor* object;
            rpm_sensor_frequency_t lastValue;
            Timer age;
            bool ageStarted;
        };

        _rpmSensorStruct_t _frontLeftWheel,
                           _frontRightWheel,
                           _rearLeftWheel,
                           _rearRightWheel;

        bool _asrActive = false;

        struct _asrSave {
            float lastTorque, lastA;
            Timer lastRun = Timer();
        } _asrSave;

        void _checkErrors() {
            if (_motorController->getStatus() > 0) {
                carService->addError(Error(ID::getComponentId(_motorController->getTelegramTypeId(), _motorController->getComponentId()), _motorController->getStatus(), ERROR_CRITICAL));
            }

            if (_asrActive) {
                if (_frontLeftWheel.object->getStatus() > 0 ||
                    _frontRightWheel.object->getStatus() > 0 ||
                    _rearLeftWheel.object->getStatus() > 0 ||
                    _rearRightWheel.object->getStatus() > 0 ||
                    _getAge(_frontLeftWheel) > STD_AGE_LIMIT ||
                    _getAge(_frontRightWheel) > STD_AGE_LIMIT ||
                    _getAge(_rearLeftWheel) > STD_AGE_LIMIT ||
                    _getAge(_rearRightWheel) > STD_AGE_LIMIT) {
                    _asrError();
                }
            }

            if ((_gasPedal.object->getStatus() > 0) || (_getAge(_gasPedal) > STD_AGE_LIMIT)) {
                _pedalError(_gasPedal.object);
            }

            if ((_brakePedal.object->getStatus() > 0) || (_getAge(_brakePedal) > STD_AGE_LIMIT)) {
                _pedalError(_brakePedal.object);
            }

            carService.run();
            if (carService.getState() == READY_TO_DRIVE) {
                _ready = true;
            } else {
                _ready = false;
            }
        }

        void _pedalError(IPedal* sensorId) {
            carService->addError(Error(ID::getComponentId(sensorId->getTelegramTypeId(), sensorId->getComponentId()), sensor->getStatus(), ERROR_CRITICAL));
        }

        void _asrError() {
            _asrActive = false;
            carService->addError(Error(ID::getComponentId(SYSTEM, SYSTEM_MASTER), 0x0, ERROR_ISSUE));
        }

        float _getAge(_rpmSensorStruct_t &sensor) {
            if (sensor.ageStarted) {
                return sensor.age.read();
            } else {
                return 0;
            }
        }

        float _getAge(_pedalStruct_t &sensor) {
            if (sensor.ageStarted) {
                return sensor.age.read();
            } else {
                return 0;
            }
        }

        void _updateValues() {
            _update(_gasPedal);
            _update(_brakePedal);

            _update(_frontLeftWheel);
            _update(_frontRightWheel);
            _update(_rearLeftWheel);
            _update(_rearRightWheel);
        }

        void _update(_pedalStruct_t &pedal) {
            // Update value and age of Pedal
            pedal_value_t newPedalValue = pedal.object->getValue();
            if ((newPedalValue != pedal.lastValue) || newPedalValue == 0) {
                if (pedal.ageStarted) {
                    pedal.age.reset();
                } else {
                    pedal.ageStarted = true;
                    pedal.age.reset();
                    pedal.age.start();
                }

                pedal.lastValue = newPedalValue;
            }
        }

        void _update(_rpmSensorStruct_t &rpmSensor) {
            // Update value and age of RPM Sensor
            pedal_value_t newRpmSensorValue = rpmSensor.object->getValue();
            if ((newRpmSensorValue != rpmSensor.lastValue) || newRpmSensorValue == 0) {
                if (rpmSensor.ageStarted) {
                    rpmSensor.age.reset();
                } else {
                    rpmSensor.ageStarted = true;
                    rpmSensor.age.reset();
                    rpmSensor.age.start();
                }

                rpmSensor.lastValue = newRpmSensorValue;
            }
        }

        void _setBasicComponents(IMotorController* motorController, IPedal* gasPedal, IPedal* brakePedal) {
            _motorController = motorController;
            _gasPedal.object = gasPedal;
            _brakePedal.object = brakePedal;
        }

        void _setASRComponents(IRpmSensor* frontLeftWheel, IRpmSensor* frontRightWheel, IRpmSensor* rearLeftWheel, IRpmSensor* rearRightWheel) {
            _asrActive = true;
            _asrSave.lastRun.stop();
            _asrSave.lastRun.reset();
            _frontLeftWheel.object = frontLeftWheel;
            _frontRightWheel.object = frontRightWheel;
            _rearLeftWheel.object = rearLeftWheel;
            _rearRightWheel.object = rearRightWheel;
        }

        pedal_value_t _getPedalPower() {
            pedal_value_t returnValue = _gasPedal.lastValue;

            if (_brakePedal.lastValue >= STD_BRAKE_POWER_LOCK_THRESHHOLD) {
                returnValue = 0;
            }

            return returnValue;
        }

        float _mapToPowerLimit(float returnValue) {
            /*  We have to values:
                _power.max:             The maximum Power to be applied on our Motor (absolut)
                _power.setOnController: The Power Maximum set on our Motor Controller.

                If the Power set on the Motor controller is lower or equal max, it will be limited to the
                one set on the Motor Controller (obvious). Else, we have to map our value.
            */

           if (_power.max < _power.setOnController) {
               returnValue = (_power.max / _power.setOnController) * returnValue;
           }

           return returnValue;
        }

        void _ASR(float returnValue) {
            return returnValue;
        }
};

#endif // MOTORCONTROLLERSERVICE_H