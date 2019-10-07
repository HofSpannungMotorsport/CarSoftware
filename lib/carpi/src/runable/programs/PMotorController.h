#ifndef PMOTORCONTROLLER_H
#define PMOTORCONTROLLER_H

#include "IProgram.h"

#include "services/SCar.h"
#include "components/interface/IMotorController.h"
#include "components/interface/IPedal.h"
#include "components/interface/IRpmSensor.h"
#include "components/software/SoftwareRpmSensor.h"

#define STD_MAX_POWER 80 // kW
#define STD_POWER_SET_ON_MOTOR_CONTROLLER 80 // kW

#define STD_AGE_LIMIT 3.0 // s
#define STD_BRAKE_POWER_LOCK_THRESHHOLD 0.40 // 40% -> if brake is put down only this amount, the Gas Pedal will be blocked

// FSG Rules relevant
// EV2.3 -> APPS / Brake Pedal Plausibility Check
#define STD_GAS_PEDAL_PRIME_MIN 0.05 // 5% -> Gas Pedal has to be lower than that to be primed (if unprimed)
#define STD_HARD_BRAKE_THRESHHOLD 0.75 // 75% (it has to be 30 bar pressure in the brake circuit, but we don't have a Sensor connected to our Microcontrollers)
#define STD_HARD_BRAKE_PRESSURE 30 // bar [il]
#define STD_HARD_BRAKE_CUTOFF_TIME 0.5 // 500 ms -> unprime gas pedal if braked hard for longer than this
#define STD_HARD_BRAKE_CUTOFF_APPS_POSITION 0.25 // 25% -> If equal or higher than that while hard brake, gas pedal will be unprimed
#define STD_HARD_BRAKE_CUTOFF_POWER 25 // kW -> If Power at Output highter than that while hard brake, gas pedal will be unprimed

class PMotorController : public IProgram {
    public:
        PMotorController(SCar &carService,
                               IMotorController* motorController,
                               IPedal* gasPedal, IPedal* brakePedal)
            : _carService(carService) {
            _setBasicComponents(motorController, gasPedal, brakePedal);
        }

        PMotorController(SCar &carService,
                               IMotorController* motorController,
                               IPedal* gasPedal, IPedal* brakePedal,
                               IRpmSensor* frontLeftWheel, IRpmSensor* frontRightWheel, IRpmSensor* rearLeftWheel, IRpmSensor* rearRightWheel)
            : _carService(carService) {
            _setBasicComponents(motorController, gasPedal, brakePedal);
            _setASRComponents(frontLeftWheel, frontRightWheel, rearLeftWheel, rearRightWheel);
        }

        virtual void run() {
            // At first, get all the new Values from the Components
            _updateValues();

            // Then, check for errors and register all of them in the SCar.
            // Also check for outdated Values (maybe because of CAN Disconnection)
            // and register it as error too.
            _checkErrors();

            float returnValue = 0;
            // Only if ready, set calculated Power
            if (_ready) {
                // Get pedal status (if brake is pushed -> gas pedal will be locked -> returns 0)
                returnValue = (float)_getPedalPower();

                if (_asrActive) {
                    returnValue = _ASR(returnValue);
                }

                // Map the Value to match the power limit/set power
                returnValue = _mapToPowerLimit(returnValue);
            } else {
                unprimeGas();
            }

            // Send new Power to Motor -> Brum Brum (but without the Brum Brum)
            // ...maybe a drivers scream ;)

            returnValue = _applyGasCurve(returnValue);

            _motorController->setTorque(returnValue);

            #if defined(MOTORCONTROLLER_OUTPUT) && defined(MESSAGE_REPORT)
                pcSerial.printf("%f\n", returnValue);
            #endif
        }

        void unprimeGas() {
            _gasPedalPrimed = false;
        }

    protected:
        SCar &_carService;
        IMotorController* _motorController;
        bool _ready = false;
        bool _communicationStarted = false;

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

        bool _gasPedalPrimed = false;
        Timer _hardBrakeingSince;
        bool _hardBrakeingStarted = false;

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
            Timer lastRun;
        } _asrSave;

        void _checkErrors() {
            if (_motorController->getStatus() > 0) {
                _carService.addError(Error(_motorController->getComponentId(), _motorController->getStatus(), ERROR_CRITICAL));
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

            _carService.run();
            if (_carService.getState() == READY_TO_DRIVE) {
                _ready = true;
            } else {
                _ready = false;
            }
        }

        void _pedalError(IPedal* sensorId) {
            _carService.addError(Error(sensorId->getComponentId(), sensorId->getStatus(), ERROR_ISSUE));
            _carService.calibrationNeeded();
        }

        void _asrError() {
            _asrActive = false;
            _carService.addError(Error(componentId::getComponentId(COMPONENT_SYSTEM, COMPONENT_SYSTEM_MASTER), 0x0, ERROR_ISSUE));
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

            if (_asrActive) {
                _update(_frontLeftWheel);
                _update(_frontRightWheel);
                _update(_rearLeftWheel);
                _update(_rearRightWheel);
            }

            if (!_communicationStarted) {
                _motorController->beginCommunication();
                _communicationStarted = true;
            }
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
            pedal_value_t newRpmSensorValue = rpmSensor.object->getFrequency();
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

            /*
                Because of FSG Rules, the APPS (gas pedal) has to be locked if
                    - APPS >= 25% || Motor Output >= 5 kW
                &&  - Hard Brakeing >= 500ms

                To Unlock it, the Pedal has to be returned to (regardlessly if brakeing or not)
                    - APPS < 5%

                All Values are set by defines at the top of this file
            */
            if (_brakePedal.lastValue >= STD_HARD_BRAKE_THRESHHOLD) {
                // -> Brake Pedal Position == Hard Brakeing
                if (_hardBrakeingStarted) {
                    // -> Hard Brakeing already before
                    if (_gasPedalPrimed) {
                        // -> APPS (gas pedal) primed -> active
                        // (otherwise no action needed)
                        if (_hardBrakeingSince.read() >= STD_HARD_BRAKE_CUTOFF_TIME) {
                            // -> Hard Brakeing too long -> it is interpreted as a Hard Brake
                            if (_gasPedal.lastValue >= STD_HARD_BRAKE_CUTOFF_APPS_POSITION) {
                                // -> Still giving Power throu the Pedal -> Pedal Position too high
                                _gasPedalPrimed = false;
                            }

                            float currentPower = _mapToPowerLimit(returnValue) * _power.setOnController;
                            if (currentPower >= STD_HARD_BRAKE_CUTOFF_POWER) {
                                // Calculated Power Output too high -> also unprime the APPS/Gas Pedal
                                _gasPedalPrimed = false;
                            }
                        }
                    }
                } else {
                    // -> Hard brakeing just started -> start counting...
                    _hardBrakeingStarted = true;
                    _hardBrakeingSince.reset();
                    _hardBrakeingSince.start();
                }
            } else {
                // -> Not hard Brakeing
                if (_hardBrakeingStarted) {
                    // -> Hard brakeing before, but not anymore -> reset
                    // If APPS (gas pedal) was unprimed before throu this,
                    // it will be unlocked at the next Part of this Method
                    _hardBrakeingStarted = false;
                }
            }

            // Check if pedal is primed, otherwise lock it
            if (!_gasPedalPrimed) {
                if (returnValue <= STD_GAS_PEDAL_PRIME_MIN) {
                    _gasPedalPrimed = true;
                } else {
                    returnValue = 0;
                }
            }

            // If brakeing, no current should go to the Motor
            if (_brakePedal.lastValue >= STD_BRAKE_POWER_LOCK_THRESHHOLD) {
                returnValue = 0;
            }

            return returnValue;
        }

        float _mapToPowerLimit(float returnValue) {
            /*  We have two values:
                _power.max:             The maximum Power to be applied on our Motor (absolut)
                _power.setOnController: The Power Maximum set on our Motor Controller.

                If the Power set on the Motor controller is lower or equal max, it will be limited to the
                one set on the Motor Controller (obvious). Else, we have to map our value.
            */

            if (_power.max < _power.setOnController) {
                returnValue = returnValue * _power.max / _power.setOnController;
            }

            return returnValue;
        }

        float _ASR(float returnValue) {
            // Implementing later
            // [il]
            return returnValue;
        }

        float _applyGasCurve(float pedalPosition) {
            float gasValue = pedalPosition * pedalPosition;

            if (gasValue > 1.0) gasValue = 1.0;
            else if (gasValue < 0.0) gasValue = 0.0;

            return gasValue;
        }
};

#endif // PMOTORCONTROLLER_H