#ifndef PMOTORCONTROLLER_H
#define PMOTORCONTROLLER_H

#include "IProgram.h"

#include "services/SCar.h"
#include "components/interface/IMotorController.h"
#include "components/interface/IPedal.h"
#include "components/interface/IRpmSensor.h"
#include "components/software/SoftwareRpmSensor.h"
#include "runable/services/SSpeed.h"

#define STD_AGE_LIMIT 3.0 // s
#define STD_BRAKE_POWER_LOCK_THRESHHOLD 0.40 // 40% -> if brake is put down only this amount, the Gas Pedal will be blocked

// FSG Rules relevant
// EV2.3 -> APPS / Brake Pedal Plausibility Check
#define STD_GAS_PEDAL_PRIME_MIN 0.05 // 5% -> Gas Pedal has to be lower than that to be primed (if unprimed)
#define STD_HARD_BRAKE_THRESHHOLD 0.75 // 75% (it has to be 30 bar pressure in the brake circuit, but we don't have a Sensor connected to our Microcontrollers)
#define STD_HARD_BRAKE_PRESSURE 30 // bar [il]
#define STD_HARD_BRAKE_CUTOFF_TIME 0.5 // 500 ms -> unprime gas pedal if braked hard for longer than this
#define STD_HARD_BRAKE_CUTOFF_APPS_POSITION 0.25 // 25% -> If equal or higher than that while hard brake, gas pedal will be unprimed

// HAS TO BE BOUNT TO SSPEED!!! The settings there define the max Current allowed, based on that, make the choice here
#define STD_MAX_RECUPERATION_PERCENTAGE 0.204 // % (245 A set Max at SSpeed -> ~50 A)
#define STD_RECU_SPEED_THRESHHOLD 2 // km/h - Under this speed, Recuperation is disabled
#define STD_RECU_SPEED_FULL 4 // km/h - From this speed up, recuperation is completely enabled

// Recuperation using the Brake Pedal. Only used, if "PMOTORCONTROLLER_USE_BRAKE_FOR_RECUPERATION" is defined
#ifdef PMOTORCONTROLLER_USE_BRAKE_FOR_RECUPERATION
    #define STD_BRAKE_RECU_START 0.05 // % where the Brake-Pedal will activate recuperation
    #define STD_BRAKE_RECU_MAX 0.20 // % where the Brake-Pedal reaches recuperation max
#else
    #define STD_GAS_RECU_THRESHHOLD 0.10 // % where the gas pedal will enter recuperation down below and power up above
#endif

class PMotorController : public IProgram {
    public:
        PMotorController(SCar &carService,
                               IMotorController* motorController,
                               IPedal* gasPedal, IPedal* brakePedal,
                               SSpeed &speedService)
            : _carService(carService), _speedService(speedService) {
            _setBasicComponents(motorController, gasPedal, brakePedal);
        }

        virtual void run() {
            // At first, get all the new Values from the Components
            _updateValues();

            // Then, check for errors and register all of them in the SCar.
            // Also check for outdated Values (maybe because of CAN Disconnection)
            // and register it as error too.
            _checkErrors();


            float returnValue = 0;
            float speed = _speedService.getSpeed();

            // Only if ready, set calculated Power
            if (_ready) {
                // Get pedal status (if brake is pushed -> gas pedal will be locked -> returns 0)
                returnValue = (float)_getPedalPower();
            } else {
                unprimeGas();
            }

            #ifdef PMOTORCONTROLLER_ACTIVATE_RECUPERATION
                #ifdef PMOTORCONTROLLER_USE_BRAKE_FOR_RECUPERATION
                    if (returnValue <= 0.00001) { // just to correct float error
                        float brakePosition = _pedal.brake->getValue();
                        if (brakePosition > STD_BRAKE_RECU_START) {
                            if (brakePosition > STD_BRAKE_RECU_MAX) {
                                returnValue = -STD_MAX_RECUPERATION_PERCENTAGE;
                            } else {
                                returnValue = -STD_MAX_RECUPERATION_PERCENTAGE * _map(brakePedal, STD_BRAKE_RECU_START, STD_BRAKE_RECU_MAX, 0.0, 1.0);
                            }
                        } else {
                            returnValue = 0;
                        }
                    }
                #else
                    if (returnValue < STD_GAS_RECU_THRESHHOLD) {
                        returnValue = _map(returnValue, 0.0, STD_GAS_RECU_THRESHHOLD, -STD_MAX_RECUPERATION_PERCENTAGE, 0.0);
                        if (returnValue < -STD_MAX_RECUPERATION_PERCENTAGE) returnValue = -STD_MAX_RECUPERATION_PERCENTAGE;
                        if (returnValue > 0.0) returnValue = 0.0;
                    } else {
                        returnValue = _map(returnValue, STD_GAS_RECU_THRESHHOLD, 1.0, 0.0, 1.0);
                        if (returnValue < 0.0) returnValue = 0.0;
                        if (returnValue > 1.0) returnValue = 1.0;
                    }
                #endif
            #endif


            if (returnValue >= 0) {
                returnValue = _applyGasCurve(returnValue);
                returnValue = _setLaunchControl(returnValue);
            }


            #ifdef PMOTORCONTROLLER_ACTIVATE_RECUPERATION
                // If Recuperating, apply speed settings
                if (returnValue < 0) {
                    if (speed < STD_RECU_SPEED_THRESHHOLD) {
                        // Too slow for recu -> disable it
                        returnValue = 0;
                    } else {
                        if (speed < STD_RECU_SPEED_FULL) {
                            // Throttle recu according to the speed
                            float maxRecu = _map(speed, STD_RECU_SPEED_THRESHHOLD, STD_RECU_SPEED_FULL, 0.0, 1.0);
                            returnValue *= maxRecu;
                        } else {
                            // Recu full enabled -> no limit needed
                        }
                    }

                    if (returnValue < -STD_MAX_RECUPERATION_PERCENTAGE)
                        returnValue = -STD_MAX_RECUPERATION_PERCENTAGE;
                }
            #else
                if (returnValue < 0) returnValue = 0;
            #endif

            returnValue *= _carService.getPowerSetting();

            #ifndef PMOTORCONTROLLER_DISABLE_MOTOR_POWER_OUTPUT
                _motorController->setTorque(returnValue);
            #else
                _motorController->setTorque(0);
            #endif

            #ifdef MOTORCONTROLLER_OUTPUT
                pcSerial.printf("%f\n", returnValue);
            #endif
        }

        void unprimeGas() {
            _gasPedalPrimed = false;
        }

    protected:
        SCar &_carService;
        SSpeed &_speedService;
        IMotorController* _motorController;
        bool _ready = false;
        bool _communicationStarted = false;

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

        float _map(float x, float in_min, float in_max, float out_min, float out_max) {
            return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
        }

        void _checkErrors() {
            if (_motorController->getStatus() > 0) {
                _carService.addError(Error(_motorController->getComponentId(), _motorController->getStatus(), ERROR_CRITICAL));
            }

            if ((_gasPedal.object->getStatus() > 0) || (_getAge(_gasPedal) > STD_AGE_LIMIT)) {
                _pedalError(_gasPedal.object);
            }

            if ((_brakePedal.object->getStatus() > 0) || (_getAge(_brakePedal) > STD_AGE_LIMIT)) {
                _pedalError(_brakePedal.object);
            }

            _carService.run();
            if (_carService.getState() == READY_TO_DRIVE || _carService.getState() == LAUNCH_CONTROL) {
                _ready = true;
            } else {
                _ready = false;
            }
        }

        void _pedalError(IPedal* sensorId) {
            _carService.addError(Error(sensorId->getComponentId(), sensorId->getStatus(), ERROR_ISSUE));
            _carService.calibrationNeeded();
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

        float _applyGasCurve(float pedalPosition) {
            float gasValue = pedalPosition;

            gas_curve_t gasCurve = _carService.getGasCurve();

            if (gasCurve == GAS_CURVE_LINEAR) {
                gasValue = pedalPosition;
            } else if (gasCurve == GAS_CURVE_X_POW_2) {
                gasValue = pedalPosition * pedalPosition;
            } else if (gasCurve == GAS_CURVE_X_POW_3) {
                gasValue = pedalPosition * pedalPosition * pedalPosition;
            } else if (gasCurve == GAS_CURVE_X_POW_4) {
                gasValue = pedalPosition * pedalPosition * pedalPosition * pedalPosition;
            }

            if (gasValue > 1.0) gasValue = 1.0;
            else if (gasValue < 0.0) gasValue = 0.0;

            return gasValue;
        }

        float _setLaunchControl(float pedalPosition) {
            if (_carService.getState() == LAUNCH_CONTROL) {
                return 0.0;
            }

            return pedalPosition;
        }
};

#endif // PMOTORCONTROLLER_H