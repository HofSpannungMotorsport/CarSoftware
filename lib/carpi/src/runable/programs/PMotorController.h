#ifndef PMOTORCONTROLLER_H
#define PMOTORCONTROLLER_H

#include "IProgram.h"

#include "services/SCar.h"
#include "components/interface/IMotorController.h"
#include "components/interface/IPedal.h"
#include "components/interface/IRpmSensor.h"
#include "runable/services/SSpeed.h" // Only for Value-Constants
#include "runable/services/SLed.h"   // Only for Value-Constants
#include "services/PowerModes.h"

#define STD_PEDAL_VALUE_AGE_LIMIT 0.3        // s
#define STD_BRAKE_POWER_LOCK_THRESHHOLD 0.40 // 40% -> if brake is put down only this amount, the Gas Pedal will be blocked

// FSG Rules relevant
// EV2.3 -> APPS / Brake Pedal Plausibility Check
#define STD_GAS_PEDAL_PRIME_MIN 0.05             // 5% -> Gas Pedal has to be lower than that to be primed (if unprimed)
#define STD_HARD_BRAKE_THRESHHOLD 0.75           // 75% (it has to be 30 bar pressure in the brake circuit, but we don't have a Sensor connected to our Microcontrollers)
#define STD_HARD_BRAKE_PRESSURE 30               // bar [il]
#define STD_HARD_BRAKE_CUTOFF_TIME 0.5           // 500 ms -> unprime gas pedal if braked hard for longer than this
#define STD_HARD_BRAKE_CUTOFF_APPS_POSITION 0.25 // 25% -> If equal or higher than that while hard brake, gas pedal will be unprimed

#define STD_RECU_SPEED_THRESHHOLD 2 // km/h - Under this speed, Recuperation is disabled
#define STD_RECU_SPEED_FULL 4       // km/h - From this speed up, recuperation is completely enabled

// Recuperation using the Brake Pedal. Only used, if "PMOTORCONTROLLER_USE_BRAKE_FOR_RECUPERATION" is defined
#ifdef PMOTORCONTROLLER_USE_BRAKE_FOR_RECUPERATION
#define STD_BRAKE_RECU_START 0.15 // % where the Brake-Pedal will activate recuperation
#define STD_BRAKE_RECU_MAX 0.25   // % where the Brake-Pedal reaches recuperation max
#else
#define STD_GAS_RECU_THRESHHOLD 0.15 // % where the gas pedal will enter recuperation down below and power up above
#endif

// Age Values
#define INVERTER_MAX_VOLTAGE_AGE 0.1 // s
#define INVERTER_MAX_SPEED_AGE 0.1   // s

// Values to Calculate the Power for the Adaptive Power Control
#define ACCU_MIN_VOLTAGE 290 // V -> is only used, if the Voltage got from Inverter is too old
#define MOTOR_MAX_RPM 6600   // rpm -> is only used, if the RPM got from Inverter is too old
// Power, Current and Torque Settings moved to PowerModes.h in services/
#define INVERTER_MAX_ALLOWED_PHASE_CURRENT 300.0 // A -> based on inverter settings
#define MOTOR_KN 0.0478                          // Vrms/RPM
#define MOTOR_I_TO_TORQUE 0.75                   // Nm/Aph rms
#define CURRENT_REGULATOR_REPORT_INTERVAL 2.0    // s

// Recuperation
// #define RECU_USE_GAS_CURVE // Applies the Curve for positive power also for negative (recuperation) power
#define RECU_MAX_ALLOWED_POWER 20160 // W -> 20,16 kW
#define RECU_MAX_ALLOWED_CURRENT 18  // A
#define RECU_MAX_ALLOWED_TORQUE 10   // Nm

// Basic ASR
#define ASR_MAX_SPEED_AGE 0.1           // s -> If any speed Value is older than this, ASR is deactivated
#define ASR_MIN_SPEED_ENTER 2           // km/h -> Before ASR is Off
#define ASR_MIN_SPEED_EXIT 3            // km/h -> After ASR gets activated
#define ASR_MIN_SPEED_LOWER_BOUNDARY 5  // km/h -> the Speed the Transfer Function gets applied if front Wheels under min speed
#define ASR_MIN_SPEED_UPPER_BOUNDARY 10 // km/h -> The Speed the Output is 0 if the front Wheels under min speed
#define ASR_LOWER_BOUNDARY 1.10         // % -> if rear axel is this amount faster then front, ASR begins throttleing
#define ASR_UPPER_BOUNDARY 1.18         // % -> if rear axel is this amount faster then front, Motor Output is 0
#define ASR_MAX_TORQUE_REDUCTION 0.95   // % -> The maximum reduction of torque if ASR is throttleing completely
#define ASR_TRANSFER_FUNCTION(x) (x * x)

// Gets calculated at compilation
#define ROOT_3 1.73205080757 // Needed for the chain factor
#define ROOT_2 1.41421356237 // Needed for AC to DC conversion

class PMotorController : public IProgram
{
public:
    PMotorController(SCar &carService, SLed &ledService,
                     IMotorController *motorController,
                     IPedal *gasPedal, IPedal *brakePedal,
                     IRpmSensor *_rpmFrontLeft, IRpmSensor *_rpmFrontRight,
                     SSpeed &speedService)
        : _carService(carService), _ledService(ledService), _speedService(speedService)
    {
        _motorController = motorController;
        _gasPedal = gasPedal;
        _brakePedal = brakePedal;
        _rpm.frontLeft = _rpmFrontLeft;
        _rpm.frontRight = _rpmFrontRight;

#ifdef PMOTORCONTROLLER_PRINT_CURRENTLY_MAX_CURRENT
        _regulatorReportTimer.reset();
        _regulatorReportTimer.start();
#endif
    }

    PMotorController(SCar &carService, SLed &ledService,
                     IMotorController *motorController,
                     IPedal *gasPedal, IPedal *brakePedal,
                     SSpeed &speedService)
        : _carService(carService), _ledService(ledService), _speedService(speedService)
    {
        _motorController = motorController;
        _gasPedal = gasPedal;
        _brakePedal = brakePedal;

#ifdef PMOTORCONTROLLER_PRINT_CURRENTLY_MAX_CURRENT
        _regulatorReportTimer.reset();
        _regulatorReportTimer.start();
#endif
    }

    virtual void run()
    {
        // At first, get all the new Values from the Components
        _updateValues();

        // Then, check for errors and register all of them in the SCar.
        // Also check for outdated Values (maybe because of CAN Disconnection)
        // and register it as error too.
        _checkErrors();

        float returnValue = 0;
        float speed = _speedService.getSpeed();

        // Only if ready, set calculated Power
        if (_ready)
        {
            // Get pedal status (if brake is pushed -> gas pedal will be locked -> returns 0)
            returnValue = (float)_getPedalPower();
        }
        else
        {
            unprimeGas();
        }

#ifdef PMOTORCONTROLLER_ACTIVATE_RECUPERATION
#ifdef PMOTORCONTROLLER_USE_BRAKE_FOR_RECUPERATION
        if (returnValue <= 0.00001)
        { // just to correct float error
            pedal_value_t brakePosition = _brakePedal->getValue();
            if (brakePosition > STD_BRAKE_RECU_START)
            {
                if (brakePosition > STD_BRAKE_RECU_MAX)
                {
                    returnValue = -1.0f;
                }
                else
                {
                    returnValue = _map(brakePosition, STD_BRAKE_RECU_START, STD_BRAKE_RECU_MAX, 0.0f, -1.0f);
                }
            }
            else
            {
                returnValue = 0.0f;
            }
        }
#else
        if (returnValue < STD_GAS_RECU_THRESHHOLD)
        {
            returnValue = _map(returnValue, 0.0f, STD_GAS_RECU_THRESHHOLD, -1.0f, 0.0f);
            if (returnValue < -1.0f)
                returnValue = -1.0f;
            if (returnValue > 0.0f)
                returnValue = 0.0f;
        }
        else
        {
            returnValue = _map(returnValue, STD_GAS_RECU_THRESHHOLD, 1.0f, 0.0f, 1.0f);
            if (returnValue < 0.0f)
                returnValue = 0.0f;
            if (returnValue > 1.0f)
                returnValue = 1.0f;
        }
#endif
#endif

        if (returnValue >= 0.0f)
        {
            returnValue = _applyGasCurve(returnValue);
            returnValue = _setLaunchControl(returnValue);
        }
#ifdef RECU_USE_GAS_CURVE
        else
        {
            returnValue = _applyNegativeGasCurve(returnValue);
        }
#endif

#ifdef PMOTORCONTROLLER_ACTIVATE_RECUPERATION
        // If Recuperating, apply speed settings
        if (returnValue < 0.0f)
        {
            if (speed < STD_RECU_SPEED_THRESHHOLD)
            {
                // Too slow for recu -> disable it
                returnValue = 0;
            }
            else
            {
                if (speed < STD_RECU_SPEED_FULL)
                {
                    // Throttle recu according to the speed
                    float maxRecu = _map(speed, STD_RECU_SPEED_THRESHHOLD, STD_RECU_SPEED_FULL, 0.0, 1.0);
                    returnValue *= maxRecu;
                }
                else
                {
                    // Recu full enabled -> no limit needed
                }
            }

            if (returnValue < -1.0f)
                returnValue = -1.0f;
        }
#else
        if (returnValue < 0.0f)
            returnValue = 0.0f;
#endif

        if (returnValue >= 0.0f)
        {
#ifdef ENABLE_POWER_MENU
            returnValue *= _getMaxAmpere(_carService.getMaxAmpere(), _carService.getMaxPower(), _carService.getMaxTorque());
#else
            returnValue *= _getMaxAmpere(MODE_0_ACCU_MAX_ALLOWED_CURRENT, MODE_0_ACCU_MAX_ALLOWED_POWER, MODE_0_MOTOR_MAX_ALLOWED_TORQUE);
#endif
        }
        else
        {
            returnValue *= _getMaxAmpere(RECU_MAX_ALLOWED_CURRENT, RECU_MAX_ALLOWED_POWER, RECU_MAX_ALLOWED_TORQUE);
        }

#ifdef EXPERIMENTELL_ASR_ACTIVE
        if (_carService.getAsrOn())
        {
            if (returnValue >= 0)
            {
                float maxTorqueByASR = _getMaxTorqueByASR();

                if (returnValue > maxTorqueByASR)
                    returnValue = maxTorqueByASR;
            }
        }
#endif

        // Age Errors
        if (_brakePedal->getValueAge() > STD_PEDAL_VALUE_AGE_LIMIT)
        {
            _pedalAgeError(_brakePedal);
            returnValue = 0;
        }

        if (_gasPedal->getValueAge() > STD_PEDAL_VALUE_AGE_LIMIT)
        {
            _pedalAgeError(_gasPedal);
            returnValue = 0;
        }

#ifndef PMOTORCONTROLLER_DISABLE_MOTOR_POWER_OUTPUT
        _motorController->setTorque(returnValue);
#else
        _motorController->setTorque(0);
#endif

#ifdef MOTORCONTROLLER_OUTPUT
        pcSerial.printf("%f\n", returnValue);
#endif
    }

    void unprimeGas()
    {
        _gasPedalPrimed = false;
    }

protected:
    SCar &_carService;
    SLed &_ledService;
    SSpeed &_speedService;
    IMotorController *_motorController;
    bool _ready = false;
    bool _communicationStarted = false;

    IPedal *_gasPedal;
    IPedal *_brakePedal;

    bool _gasPedalPrimed = false;
    Timer _hardBrakeingSince;
    bool _hardBrakeingStarted = false;

#ifdef EXPERIMENTELL_ASR_ACTIVE
    bool _asrMinSpeedMode = true;
#endif

#ifdef PMOTORCONTROLLER_PRINT_CURRENTLY_MAX_CURRENT
    Timer _regulatorReportTimer;
#endif

    struct
    {
        IRpmSensor *frontLeft;
        IRpmSensor *frontRight;
    } _rpm;

    float _map(float x, float in_min, float in_max, float out_min, float out_max)
    {
        return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
    }

    void _checkErrors()
    {
        if (_motorController->getStatus() > 0)
        {
            _carService.addError(Error(_motorController->getComponentId(), _motorController->getStatus(), ERROR_CRITICAL));
        }

        if (_carService.getState() == LAUNCH_CONTROL)
        {
            _gasPedal->resetAge();
            _brakePedal->resetAge();
        }

        if (_gasPedal->getStatus() > 0)
        {
            _pedalError(_gasPedal);
        }

        if (_brakePedal->getStatus() > 0)
        {
            _pedalError(_brakePedal);
        }

        _carService.run();
        if (_carService.getState() == READY_TO_DRIVE || _carService.getState() == LAUNCH_CONTROL)
        {
            _ready = true;
        }
        else
        {
            _ready = false;
        }
    }

    void _pedalError(IPedal *sensorId)
    {
        _carService.addError(Error(sensorId->getComponentId(), sensorId->getStatus(), ERROR_ISSUE));
        _carService.calibrationNeeded();
    }

    void _pedalAgeError(IPedal *sensorId)
    {
        _carService.addError(Error(sensorId->getComponentId(), sensorId->getStatus(), ERROR_ISSUE));
        _ledService.pedalCommunitactionInterference();
    }

    void _updateValues()
    {
        if (!_communicationStarted)
        {
            _motorController->beginCommunication();
            _communicationStarted = true;
        }
    }

    pedal_value_t _getPedalPower()
    {
        pedal_value_t gasValue = _gasPedal->getValue();
        pedal_value_t returnValue = gasValue;
        pedal_value_t brakeValue = _brakePedal->getValue();

        /*
            Because of FSG Rules, the APPS (gas pedal) has to be locked if
                - APPS >= 25% || Motor Output >= 5 kW
            &&  - Hard Brakeing >= 500ms

            To Unlock it, the Pedal has to be returned to (regardlessly if brakeing or not)
                - APPS < 5%

            All Values are set by defines at the top of this file
        */
        if (brakeValue >= STD_HARD_BRAKE_THRESHHOLD)
        {
            // -> Brake Pedal Position == Hard Brakeing
            if (_hardBrakeingStarted)
            {
                // -> Hard Brakeing already before
                if (_gasPedalPrimed)
                {
                    // -> APPS (gas pedal) primed -> active
                    // (otherwise no action needed)
                    if (_hardBrakeingSince.read() >= STD_HARD_BRAKE_CUTOFF_TIME)
                    {
                        // -> Hard Brakeing too long -> it is interpreted as a Hard Brake
                        if (gasValue >= STD_HARD_BRAKE_CUTOFF_APPS_POSITION)
                        {
                            // -> Still giving Power throu the Pedal -> Pedal Position too high
                            _gasPedalPrimed = false;
                        }
                    }
                }
            }
            else
            {
                // -> Hard brakeing just started -> start counting...
                _hardBrakeingStarted = true;
                _hardBrakeingSince.reset();
                _hardBrakeingSince.start();
            }
        }
        else
        {
            // -> Not hard Brakeing
            if (_hardBrakeingStarted)
            {
                // -> Hard brakeing before, but not anymore -> reset
                // If APPS (gas pedal) was unprimed before throu this,
                // it will be unlocked at the next Part of this Method
                _hardBrakeingStarted = false;
            }
        }

        // Check if pedal is primed, otherwise lock it
        if (!_gasPedalPrimed)
        {
            if (returnValue <= STD_GAS_PEDAL_PRIME_MIN)
            {
                _gasPedalPrimed = true;
            }
            else
            {
                returnValue = 0;
            }
        }

        // If brakeing, no current should go to the Motor
        if (brakeValue >= STD_BRAKE_POWER_LOCK_THRESHHOLD)
        {
            returnValue = 0;
        }

        return returnValue;
    }

    float _applyGasCurve(float pedalPosition)
    {
        float gasValue = pedalPosition;

        gas_curve_t gasCurve = _carService.getGasCurve();

        if (gasCurve == GAS_CURVE_LINEAR)
        {
            gasValue = pedalPosition;
        }
        else if (gasCurve == GAS_CURVE_X_POW_2)
        {
            gasValue = pedalPosition * pedalPosition;
        }
        else if (gasCurve == GAS_CURVE_X_POW_3)
        {
            gasValue = pedalPosition * pedalPosition * pedalPosition;
        }
        else if (gasCurve == GAS_CURVE_X_POW_4)
        {
            gasValue = pedalPosition * pedalPosition * pedalPosition * pedalPosition;
        }

        if (gasValue > 1.0f)
            gasValue = 1.0f;
        else if (gasValue < 0.0f)
            gasValue = 0.0f;

        return gasValue;
    }

    float _applyNegativeGasCurve(float pedalPosition)
    {
        float gasValue = pedalPosition;

        gas_curve_t gasCurve = _carService.getGasCurve();

        if (gasCurve == GAS_CURVE_LINEAR)
        {
            gasValue = pedalPosition;
        }
        else if (gasCurve == GAS_CURVE_X_POW_2)
        {
            gasValue = pedalPosition * pedalPosition * -1.0f;
        }
        else if (gasCurve == GAS_CURVE_X_POW_3)
        {
            gasValue = pedalPosition * pedalPosition * pedalPosition;
        }
        else if (gasCurve == GAS_CURVE_X_POW_4)
        {
            gasValue = pedalPosition * pedalPosition * pedalPosition * pedalPosition * -1.0f;
        }

        if (gasValue > 0.0f)
            gasValue = 0.0f;
        else if (gasValue < -1.0f)
            gasValue = -1.0f;

        return gasValue;
    }

    float _setLaunchControl(float pedalPosition)
    {
        if (_carService.getState() == LAUNCH_CONTROL)
        {
            return 0.0;
        }

        return pedalPosition;
    }

    float _getMaxAmpere(float maxAllowedCurrent, float maxAllowedPower, float maxAllowedTorque)
    {
        float rpmSpeed = _motorController->getSpeed();
        float dcVoltage = _motorController->getDcVoltage();

        // Only use RPM if it is new
        if (_motorController->getSpeedAge() > INVERTER_MAX_SPEED_AGE)
        {
            rpmSpeed = (float)MOTOR_MAX_RPM;
        }

        // Only use this voltage if it is new
        if (_motorController->getDcVoltageAge() > INVERTER_MAX_VOLTAGE_AGE)
        {
            dcVoltage = (float)ACCU_MIN_VOLTAGE;
        }

        float motorVoltage = rpmSpeed * (float)MOTOR_KN;
        float maxAcVoltage = dcVoltage / (float)ROOT_2;
        if (motorVoltage > maxAcVoltage)
        {
            motorVoltage = maxAcVoltage;
        }

        float maxPossiblePower = motorVoltage * (float)INVERTER_MAX_ALLOWED_PHASE_CURRENT * (float)ROOT_3;

        if (maxPossiblePower < 0.0f)
        {
            maxPossiblePower *= -1.0f;
        }

        float powerLimit = 1.0f;
        if (maxPossiblePower > 0.0f)
        {
            // Now limit the power either by the allowed current OR by the allowed Power OR by the allowed Torque
            float powerLimitByCurrent = (dcVoltage * maxAllowedCurrent) / maxPossiblePower;
            float powerLimitByPower = maxAllowedPower / maxPossiblePower;
            float powerLimitByTorque = maxAllowedTorque / ((float)INVERTER_MAX_ALLOWED_PHASE_CURRENT * (float)MOTOR_I_TO_TORQUE);

            // Check for lowest power setting
            powerLimit = _min(powerLimitByCurrent, _min(powerLimitByPower, powerLimitByTorque));

            // Limit to boundary
            if (powerLimit > 1.0f)
                powerLimit = 1.0f;
            else if (powerLimit < 0.0f)
                powerLimit = 0.0f;

#ifdef PMOTORCONTROLLER_PRINT_CURRENTLY_MAX_CURRENT
            if (_regulatorReportTimer.read() >= CURRENT_REGULATOR_REPORT_INTERVAL)
            {
                _regulatorReportTimer.reset();
                _regulatorReportTimer.start();
                pcSerial.printf("Currently Max Current: %.1f RMS\t %.2f\t %.2f\t %.2f\t %.2f\t%.0f RPM\t%.2f V DC\n", INVERTER_MAX_ALLOWED_PHASE_CURRENT * powerLimit, powerLimit, powerLimitByCurrent, powerLimitByPower, powerLimitByTorque, rpmSpeed, dcVoltage);
            }
#endif
        }

        return powerLimit;
    }

    float _min(float x, float y)
    {
        if (x < y)
            return x;
        else
            return y;
    }

#ifdef EXPERIMENTELL_ASR_ACTIVE

    float _getFrontSpeed()
    {
        // float middleRpm = (_rpm.frontLeft->getFrequency() * 0.5) + (_rpm.frontRight->getFrequency() * 0.5);

        // Because only one of the two front sensors is working, for testing, we use only one sensor
        float middleRpm = _rpm.frontRight->getFrequency();

        return middleRpm * WHEEL_RPM_TO_KMH;
    }

    float _getFrontSpeedAge()
    {
        // float left = _rpm.frontLeft->getAge();
        float right = _rpm.frontRight->getAge();

        // if (left > right)
        //     return left;
        // else
        return right;
    }

    float _getRearSpeed()
    {
        return _motorController->getSpeed() * MOTOR_RPM_TO_KMH;
    }

    float _getMaxTorqueByASR()
    {
        float frontAge = _getFrontSpeedAge();
        float rearAge = _motorController->getSpeedAge();

        if (frontAge > (float)ASR_MAX_SPEED_AGE || rearAge > (float)ASR_MAX_SPEED_AGE)
            return 1.0f;

        float frontSpeed = _getFrontSpeed();
        float rearSpeed = _getRearSpeed();

        // pcSerial.printf("%f\t%f\t%f\t%f\n", frontSpeed, rearSpeed, frontAge, rearAge);

        if (_asrMinSpeedMode)
        {
            if (frontSpeed >= (float)ASR_MIN_SPEED_EXIT)
            {
                _asrMinSpeedMode = false;
            }
        }
        else
        {
            if (frontSpeed <= (float)ASR_MIN_SPEED_ENTER)
            {
                _asrMinSpeedMode = true;
            }
        }

        float torqueReduction = 0.0f;
        if (_asrMinSpeedMode)
        {
            if (rearSpeed >= (float)ASR_MIN_SPEED_LOWER_BOUNDARY &&
                rearSpeed <= (float)ASR_MIN_SPEED_UPPER_BOUNDARY)
            {
                torqueReduction = _map(rearSpeed, ASR_MIN_SPEED_LOWER_BOUNDARY, ASR_MIN_SPEED_UPPER_BOUNDARY, 0.0f, 1.0f);
                torqueReduction = ASR_TRANSFER_FUNCTION(torqueReduction);
            }
            else if (rearSpeed > (float)ASR_MIN_SPEED_UPPER_BOUNDARY)
            {
                torqueReduction = 1.0f;
            }
        }
        else
        {
            float speedRatio = rearSpeed / frontSpeed;

            if (speedRatio >= (float)ASR_LOWER_BOUNDARY &&
                speedRatio <= (float)ASR_UPPER_BOUNDARY)
            {
                torqueReduction = _map(speedRatio, (float)ASR_LOWER_BOUNDARY, (float)ASR_UPPER_BOUNDARY, 0.0f, 1.0f);
                torqueReduction = ASR_TRANSFER_FUNCTION(torqueReduction);
            }
            else if (speedRatio > (float)ASR_UPPER_BOUNDARY)
            {
                torqueReduction = 1.0f;
            }
        }

        if (torqueReduction > 1.0f)
            torqueReduction = 1.0f;
        else if (torqueReduction < 0.0f)
            torqueReduction = 0.0f;

        return 1.0f - (torqueReduction * ASR_MAX_TORQUE_REDUCTION);
    }

#endif
};

#endif // PMOTORCONTROLLER_H