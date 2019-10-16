#ifndef PMOTORCONTROLLER_H
#define PMOTORCONTROLLER_H

#include "IProgram.h"

#include "services/SCar.h"
#include "components/interface/IMotorController.h"
#include "components/interface/IPedal.h"
#include "components/interface/IRpmSensor.h"
#include "components/software/SoftwareRpmSensor.h"

class PMotorController : public IProgram {
    public:
        PMotorController(SCar &carService, IRegistry &registry,
                         IMotorController &motorController,
                         IPedal &gasPedal, IPedal &brakePedal)
            : _carService(carService), _registry(registry),
              _motorController(motorController), _gasPedal(gasPedal), _brakePedal(brakePedal) {}

        PMotorController(SCar &carService, IRegistry &registry,
                         IMotorController &motorController,
                         IPedal &gasPedal, IPedal &brakePedal,
                         IRpmSensor &frontLeftWheel, IRpmSensor &frontRightWheel, IRpmSensor &rearLeftWheel, IRpmSensor &rearRightWheel)
            : _carService(carService), _registry(registry),
              _motorController(motorController), _gasPedal(gasPedal), _brakePedal(brakePedal) {
            _asrActive = true;
            _asrSave.lastRun.stop();
            _asrSave.lastRun.reset();

            _frontLeftWheel.object = &frontLeftWheel;
            _frontRightWheel.object = &frontRightWheel;
            _rearLeftWheel.object = &rearLeftWheel;
            _rearRightWheel.object = &rearRightWheel;
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

            returnValue = _applyMaxPower(returnValue);

            _motorController.setTorque(returnValue);

            #if defined(MOTORCONTROLLER_OUTPUT) && defined(MESSAGE_REPORT)
                pcSerial.printf("%f\n", returnValue);
            #endif
        }

        void unprimeGas() {
            _gasPedalPrimed = false;
        }

    protected:
        IRegistry &_registry;

        SCar &_carService;
        IMotorController &_motorController;
        bool _ready = false;
        bool _communicationStarted = false;

        struct _pedalStruct_t {
            _pedalStruct_t(IPedal &pedalObject) : object(pedalObject) {}

            IPedal &object;
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
            IRpmSensor* object; // Using Pointer to also be able NOT to use
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
            if (_motorController.getStatus() > 0) {
                _carService.addError(Error(_motorController.getComponentId(), _motorController.getStatus(), ERROR_CRITICAL));
            }

            float ageLimit = _registry.getFloat(PMC_AGE_LIMIT);

            if (_asrActive) {
                if (_frontLeftWheel.object->getStatus() > 0 ||
                    _frontRightWheel.object->getStatus() > 0 ||
                    _rearLeftWheel.object->getStatus() > 0 ||
                    _rearRightWheel.object->getStatus() > 0 ||
                    _getAge(_frontLeftWheel) > ageLimit ||
                    _getAge(_frontRightWheel) > ageLimit ||
                    _getAge(_rearLeftWheel) > ageLimit ||
                    _getAge(_rearRightWheel) > ageLimit) {
                    _asrError();
                }
            }

            if ((_gasPedal.object.getStatus() > 0) || (_getAge(_gasPedal) > ageLimit)) {
                _pedalError(_gasPedal.object);
            }

            if ((_brakePedal.object.getStatus() > 0) || (_getAge(_brakePedal) > ageLimit)) {
                _pedalError(_brakePedal.object);
            }

            _carService.run();
            if (_carService.getState() == READY_TO_DRIVE) {
                _ready = true;
            } else {
                _ready = false;
            }
        }

        void _pedalError(IPedal &sensorId) {
            _carService.addError(Error(sensorId.getComponentId(), sensorId.getStatus(), ERROR_ISSUE));
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
                _motorController.beginCommunication();
                _communicationStarted = true;
            }
        }

        void _update(_pedalStruct_t &pedal) {
            // Update value and age of Pedal
            pedal_value_t newPedalValue = pedal.object.getValue();
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
            if (_brakePedal.lastValue >= _registry.getFloat(PMC_HARD_BRAKE_THRESHHOLD)) {
                // -> Brake Pedal Position == Hard Brakeing
                if (_hardBrakeingStarted) {
                    // -> Hard Brakeing already before
                    if (_gasPedalPrimed) {
                        // -> APPS (gas pedal) primed -> active
                        // (otherwise no action needed)
                        if (_hardBrakeingSince.read() >= _registry.getFloat(PMC_HARD_BRAKE_CUTOFF_TIME)) {
                            // -> Hard Brakeing too long -> it is interpreted as a Hard Brake
                            if (_gasPedal.lastValue >= _registry.getFloat(PMC_HARD_BRAKE_CUTOFF_APPS_POSITION)) {
                                // -> Still giving Power throu the Pedal -> Pedal Position too high
                                _gasPedalPrimed = false;
                            }

                            float currentPower = _mapToPowerLimit(returnValue) * _registry.getFloat(PMC_POWER_SET_ON_MC);
                            if (currentPower >= _registry.getFloat(PMC_HARD_BRAKE_CUTOFF_POWER)) {
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
                if (returnValue <= _registry.getFloat(PMC_GAS_PEDAL_PRIME_MIN)) {
                    _gasPedalPrimed = true;
                } else {
                    returnValue = 0;
                }
            }

            // If brakeing, no current should go to the Motor
            if (_brakePedal.lastValue >= _registry.getFloat(PMC_BRAKE_POWER_LOCK_THRESHHOLD)) {
                returnValue = 0;
            }

            return returnValue;
        }

        float _mapToPowerLimit(float returnValue) {
            /*  We have two values:
                powerMax:             The maximum Power to be applied on our Motor (absolut)
                powerSetOnController: The Power Maximum set on our Motor Controller.

                If the Power set on the Motor controller is lower or equal max, it will be limited to the
                one set on the Motor Controller (obvious). Else, we have to map our value.
            */

           float powerMax = _registry.getFloat(PMC_MAX_POWER);
           float powerSetOnController = _registry.getFloat(PMC_POWER_SET_ON_MC);

            if (powerMax < powerSetOnController) {
                returnValue = returnValue * powerMax / powerSetOnController;
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

        float _applyMaxPower(float pedalPosition) {
            return pedalPosition * _carService.getMaxPower();
        }
};

#endif // PMOTORCONTROLLER_H