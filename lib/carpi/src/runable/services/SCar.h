#ifndef SCAR_H
#define SCAR_H

#include "platform/CircularBuffer.h"
#include "IService.h"
#include "runable/programs/PCockpitIndicator.h"
#include "runable/programs/PBrakeLight.h"
#include "runable/services/SLed.h"
#include "communication/componentIds.h"
#include "communication/CANService.h"
#include "components/interface/IButton.h"
#include "components/interface/ILed.h"
#include "components/interface/IBuzzer.h"
#include "components/interface/IMotorController.h"
#include "components/interface/IHvEnabled.h"
#include "PowerModes.h"

#define STARTUP_WAIT 100               // ms wait_us before system gets started
#define ERROR_REGISTER_SIZE 64         // errors, max: 255
#define STARTUP_ANIMATION_WAIT_AFTER 3 // ms wait_us after animation
#define BRAKE_START_THRESHHOLD 0.60    // %

#define MAX_BUTTON_STATE_AGE 500000 // us

#define LAUNCH_CONTROL_RELEASE_VALUE 0.95 // % where the gas is activated again

#define HV_ENABLED_BEEP_TIME 2200 // s (has to be at least 0.5)

#define BEEP_MULTI_BEEP_TIME 100000  // us
#define BEEP_MULTI_SILENT_TIME 80000 // us

#define LED_RESEND_INTERVAL_IN_RUN 330000 // s

// #define NO_ACCU

#ifdef NO_ACCU
#warning "NO_ACCU is defined"
#endif

enum gas_curve_t : uint8_t
{
    GAS_CURVE_LINEAR = 0x0,
    GAS_CURVE_X_POW_2,
    GAS_CURVE_X_POW_3,
    GAS_CURVE_X_POW_4
};

enum error_type_t : uint8_t
{
    ERROR_NO = 0x0,
    ERROR_UNDEFINED = 0x1,
    ERROR_ISSUE = 0x2,
    ERROR_SYSTEM = 0x3,  // buffer-overflow...
    ERROR_CRITICAL = 0x4 // Pedal-Error -> not ready to drive anymore
};

class Error
{
public:
    Error() {}
    Error(id_component_t ComponentId, uint8_t Code, error_type_t Type)
        : componentId(ComponentId), code(Code), type(Type) {}

    id_component_t componentId = 0;
    uint8_t code = 0;
    error_type_t type = ERROR_NO;
};

class SCar : public IService
{
public:
    SCar(CANService &canService, SLed &ledService,
         IButton *buttonReset, IButton *buttonCalibrate, IButton *buttonStart, IButton *buttonTsOn,
         IPedal *gasPedal, IPedal *brakePedal,
         IBuzzer *buzzer,
         IMotorController *motorController,
         IHvEnabled *hvEnabled,
         DigitalIn *tsms,
         PBrakeLight &brakeLightService)
        : _canService(canService), _ledService(ledService), _brakeLightService(brakeLightService)
    {
        _button.reset = buttonReset;
        _button.calibrate = buttonCalibrate;
        _button.start = buttonStart;
        _button.tsOn = buttonTsOn;
        _pedal.gas = gasPedal;
        _pedal.brake = brakePedal;

        _buzzer = buzzer;

        _motorController = motorController;

        _tsms = tsms;
        _setState(LV_NOT_CALIBRATED);
    }

    void run()
    {
        _ledService.indicateCarState(_state); // WHYYYYY???
        if (_state == HV_CALIBRATED || _state == HV_CALIBRATING || _state == HV_NOT_CALIBRATED || _state == DRIVE)
        {
            _checkHvEnabled();
        }
        checkState();

        if (_button.reset->getStatus() > 0)
        {
            addError(Error(_calculateComponentId((IComponent *)_button.reset), _button.reset->getStatus(), ERROR_SYSTEM));
        }

        if (_button.calibrate->getStatus() > 0)
        {
            addError(Error(_calculateComponentId((IComponent *)_button.calibrate), _button.calibrate->getStatus(), ERROR_SYSTEM));
        }

        if (_button.start->getStatus() > 0)
        {
            addError(Error(_calculateComponentId((IComponent *)_button.start), _button.start->getStatus(), ERROR_SYSTEM));
        }

        if (_button.tsOn->getStatus() > 0)
        {
            addError(Error(_calculateComponentId((IComponent *)_button.start), _button.start->getStatus(), ERROR_SYSTEM));
        }

        if (!(_errorRegister.empty()))
        {
            processErrors();
        }
    }

    void addError(Error error)
    {
        _errorRegister.push(error);
    }

    void processErrors()
    {
        while (!_errorRegister.empty())
        {
            Error error;
            _errorRegister.pop(error);

#ifdef REPORT_ERRORS
            pcSerial.printf("[SCar]@processErrors: Got Error at 0x%x with error code 0x%x and error type 0x%x !\n", error.componentId, error.code, error.type);
#endif

            if (error.type >= ERROR_CRITICAL)
            {
                // Critical Error
                // Red -> Fast Blinking
                // Green -> Off
                // Car -> Error -> No driving anymore, Error must be solved

                _setState(CAR_ERROR);
                _motorController->setRUN(MOTOR_CONTROLLER_RUN_DISABLE);
                _motorController->setRFE(MOTOR_CONTROLLER_RFE_DISABLE);
            }
        }
    }

    car_state_t getState()
    {
        return _state;
    }

    void _setState(car_state_t state)
    {
        _state = state;
        _ledService.indicateCarState(_state);
        if (_state == CAR_ERROR)
        {
            _beepTimes(5, 0.1, 0.1);
            _motorController->setRUN(MOTOR_CONTROLLER_RUN_DISABLE);
            _motorController->setRFE(MOTOR_CONTROLLER_RFE_DISABLE);
        }
    }

    /**
     * Pedals need to calibrate
     */
    void calibrationNeeded(bool errorByCommunication = false)
    {
        if (_state == DRIVE)
        {
            _motorController->setRUN(MOTOR_CONTROLLER_RUN_DISABLE);
            _motorController->setRFE(MOTOR_CONTROLLER_RFE_DISABLE);
            _setState(HV_NOT_CALIBRATED);
        }
        else if (_state == LV_CALIBRATED)
        {
            _setState(LV_NOT_CALIBRATED);
        }
        else if (_state == HV_CALIBRATED)
        {
            _setState(HV_NOT_CALIBRATED);
        }

        if (errorByCommunication)
        {
            _ledService._led.red->setState(LED_ON);
            _ledService._led.red->setBlinking(BLINKING_FAST);
        }

        _ledService.run();
    }

    void startUp()
    {
        wait_ms(STARTUP_WAIT);

        _ledService.startupAnimation();

        wait_ms(STARTUP_ANIMATION_WAIT_AFTER);
    }

    gas_curve_t getGasCurve()
    {
        return _gasCurve;
    }

#ifdef ENABLE_POWER_MENU
    uint8_t getCurrentModeId()
    {
        return _currentModeId;
    }

    float getMaxPower()
    {
        return maxAllowedPower;
    }

    float getMaxAmpere()
    {
        return maxAllowedAmpere;
    }

    float getMaxTorque()
    {
        return maxAllowedTorque;
    }

#ifdef EXPERIMENTELL_ASR_ACTIVE
    bool getAsrOn()
    {
        return asrOn;
    }
#endif
#endif

private:
    CANService &_canService;
    SLed &_ledService;
    CircularBuffer<Error, ERROR_REGISTER_SIZE, uint8_t>
        _errorRegister;

    car_state_t _state = CAR_OFF;

    gas_curve_t _gasCurve = GAS_CURVE_X_POW_2;

#ifdef ENABLE_POWER_MENU
    uint8_t _currentModeId = 0;
    // Settings for Power Modes (driving, recu is static)
    float maxAllowedAmpere = MODE_0_ACCU_MAX_ALLOWED_CURRENT;
    float maxAllowedPower = MODE_0_ACCU_MAX_ALLOWED_POWER;
    float maxAllowedTorque = MODE_0_MOTOR_MAX_ALLOWED_TORQUE;
#ifdef EXPERIMENTELL_ASR_ACTIVE
    bool asrOn = false;
#endif
#endif

    struct _button
    {
        IButton *reset;
        IButton *calibrate;
        IButton *start;
        IButton *tsOn;
    } _button;

    struct _pedal
    {
        IPedal *gas;
        IPedal *brake;
    } _pedal;

    IBuzzer *_buzzer;
    Timer _timer;

    IMotorController *_motorController;

    DigitalIn *_tsms;

    PBrakeLight &_brakeLightService;

    bool _isHvEnabled = false;

    id_component_t _calculateComponentId(IComponent *component)
    {
        id_component_t id = component->getComponentId();
        return id;
    }

    /**
     * Send Commands to LEDs over Can
     */
    void _sendLedsOverCan()
    {
        _ledService.run();
    }

    /**
     * Send Commands to Pedals over Can
     */
    void _sendPedalsOverCan()
    {
        // Pedals
        _canService.sendMessage((ICommunication *)_pedal.gas, DEVICE_PEDAL);
        _canService.sendMessage((ICommunication *)_pedal.brake, DEVICE_PEDAL);
    }

    /**
     * Send Commands to LEDs and Pedals over Can
     */
    void _sendComponentsOverCan()
    {
        _ledService.run();
        _sendPedalsOverCan();
    }

    void _pedalError(IPedal *sensorId)
    {
        addError(Error(sensorId->getComponentId(), sensorId->getStatus(), ERROR_ISSUE));
        calibrationNeeded();
    }

    /**
     * Check if Shutdown is closed before and after TSMS
     */
    void _checkHvEnabled()
    {
        // [QF]
        static bool hvEnabledErrorAdded = false;
        static bool tsmsErrorAdded = false;

#ifndef NO_ACCU
        if (!(_tsms->read()) || (_motorController->getDcVoltage() < 60))
#else
        if (!(_tsms->read())) // if Shutdwn after TSMS or before TSMS not closed
#endif
        {
            switch (_state)
            {
            case DRIVE:
                // change car state, disable Inverte and let Dashbaord LED blink fast
                _motorController->setRUN(MOTOR_CONTROLLER_RUN_DISABLE);
                _motorController->setRFE(MOTOR_CONTROLLER_RFE_DISABLE);
                _setState(LV_CALIBRATED);

                break;
            case HV_CALIBRATED:
                _setState(LV_CALIBRATED);
                break;
            case HV_NOT_CALIBRATED:
                _setState(LV_NOT_CALIBRATED);
                break;
            default:
                break;
            }

            // Add Shutdown after TSMS not closed Error
            if (!hvEnabledErrorAdded)
            {
                addError(Error(componentId::getComponentId(COMPONENT_SYSTEM, COMPONENT_SYSTEM_60V_OK), 0x1, ERROR_ISSUE));
                hvEnabledErrorAdded = true;
            }

            // Add Shutdown before TSMS not closed Error
            if (!tsmsErrorAdded)
            {
                addError(Error(componentId::getComponentId(COMPONENT_SYSTEM, COMPONENT_SYSTEM_TSMS), 0x1, ERROR_ISSUE));
                tsmsErrorAdded = true;
            }
        }
        else
        {
            hvEnabledErrorAdded = false;
            tsmsErrorAdded = false;
        }
    }

    /**
     * Calibrate Pedals until Start Button is pressed long
     */
    void _startCalibratePedals()
    {
        _pedal.gas->setCalibrationStatus(CURRENTLY_CALIBRATING);
        _pedal.brake->setCalibrationStatus(CURRENTLY_CALIBRATING);
        _sendPedalsOverCan();
    }

    void _stopCalibratePedals()
    {
        // Stop calibration
        _pedal.gas->setCalibrationStatus(CURRENTLY_NOT_CALIBRATING);
        _pedal.brake->setCalibrationStatus(CURRENTLY_NOT_CALIBRATING);
        _sendPedalsOverCan();

        wait_ms(100);
        _canService.processInbound();

        // Check for Errors at Calibration
        _canService.processInbound();
        if (_pedal.gas->getStatus() > 0)
        {
            _pedalError(_pedal.gas);
        }

        if (_pedal.brake->getStatus() > 0)
        {
            _pedalError(_pedal.brake);
        }

        processErrors();

        // If all OK, go into Almost Ready to drive
        if (!(_state == LV_CALIBRATED || _state == HV_CALIBRATED))
        {
            calibrationNeeded();
            return;
        }
    }

#ifdef ENABLE_POWER_MENU
    bool resetWasPressedBefore = false;
    bool resetWasNotPressedBefore = false;
#endif

    void _starWars()
    {
        const int c = 261;
        const int d = 294;
        const int e = 329;
        const int f = 349;
        const int g = 391;
        const int gS = 415;
        const int a = 440;
        const int aS = 455;
        const int b = 466;
        const int cH = 523;
        const int cSH = 554;
        const int dH = 587;
        const int dSH = 622;
        const int eH = 659;
        const int fH = 698;
        const int fSH = 740;
        const int gH = 784;
        const int gSH = 830;
        const int aH = 880;

        _buzzer->playTone(a, 500);
        _buzzer->playTone(a, 500);
        _buzzer->playTone(a, 500);
        _buzzer->playTone(f, 350);
        _buzzer->playTone(cH, 150);
        _buzzer->playTone(a, 500);
        _buzzer->playTone(f, 350);
        _buzzer->playTone(cH, 150);
        _buzzer->playTone(a, 650);

        wait_ms(500);

        _buzzer->playTone(eH, 500);
        _buzzer->playTone(eH, 500);
        _buzzer->playTone(eH, 500);
        _buzzer->playTone(fH, 350);
        _buzzer->playTone(cH, 150);
        _buzzer->playTone(gS, 500);
        _buzzer->playTone(f, 350);
        _buzzer->playTone(cH, 150);
        _buzzer->playTone(a, 650);

        wait_ms(500);

        _buzzer->playTone(aH, 500);
        _buzzer->playTone(a, 300);
        _buzzer->playTone(a, 150);
        _buzzer->playTone(aH, 500);
        _buzzer->playTone(gSH, 325);
        _buzzer->playTone(gH, 175);
        _buzzer->playTone(fSH, 125);
        _buzzer->playTone(fH, 125);
        _buzzer->playTone(fSH, 250);

        wait_ms(325);

        _buzzer->playTone(aS, 250);
        _buzzer->playTone(dSH, 500);
        _buzzer->playTone(dH, 325);
        _buzzer->playTone(cSH, 175);
        _buzzer->playTone(cH, 125);
        _buzzer->playTone(b, 125);
        _buzzer->playTone(cH, 250);

        wait_ms(350);
    }

    void checkState()
    {
        _canService.processInbound();
        processErrors();

        switch (_state)
        {
        case CAR_OFF:
            /* code */
            _setState(CAR_ERROR);
            break;

        case LV_NOT_CALIBRATED:

#ifndef NO_ACCU
            if (_tsms->read() && _motorController->getDcVoltage() > 60)
#else
            if (_tsms->read())
#endif
            {
                _setState(HV_NOT_CALIBRATED);
            }
            if (_button.calibrate->getState() == LONG_CLICKED && _button.calibrate->getStateAge() < MAX_BUTTON_STATE_AGE)
            {
                _startCalibratePedals();
                _setState(LV_CALIBRATING);
            }
            checkPowerMenu();

            break;
        case HV_NOT_CALIBRATED:
            if (_button.calibrate->getState() == LONG_CLICKED && _button.calibrate->getStateAge() < MAX_BUTTON_STATE_AGE)
            {
                _startCalibratePedals();
                _setState(HV_CALIBRATING);
            }
            checkPowerMenu();

            break;

        case LV_CALIBRATING:
            if (_button.start->getState() == LONG_CLICKED && _button.start->getStateAge() < MAX_BUTTON_STATE_AGE)
            {
                _setState(LV_CALIBRATED);
                _stopCalibratePedals();
            }
            break;

        case HV_CALIBRATING:

            if (_button.start->getState() == LONG_CLICKED && _button.start->getStateAge() < MAX_BUTTON_STATE_AGE)
            {
                _setState(HV_CALIBRATED);
                _stopCalibratePedals();
            }

            /*if (_button.reset->getState() == LONG_CLICKED && _button.reset->getStateAge() < MAX_BUTTON_STATE_AGE)
            {
                bspdTestOut.write(1);
            }*/
            break;

        case LV_CALIBRATED:

#ifndef NO_ACCU
            if (_tsms->read() && _motorController->getDcVoltage() > 60)
#else
            if (_tsms->read())
#endif

            {
                _setState(HV_CALIBRATED);
            }
            if (_button.calibrate->getState() == LONG_CLICKED && _button.calibrate->getStateAge() < MAX_BUTTON_STATE_AGE)
            {
                _startCalibratePedals();
                _setState(LV_CALIBRATING);
            }
            checkPowerMenu();

            break;

        case HV_CALIBRATED:
#ifndef NO_ACCU
            if ((_button.start->getState() == LONG_CLICKED && _button.start->getStateAge() < MAX_BUTTON_STATE_AGE) && (_pedal.brake->getValue() >= BRAKE_START_THRESHHOLD) && (_tsms->read()) && _motorController->getDcVoltage() > 60)
#else
            if ((_button.start->getState() == LONG_CLICKED && _button.start->getStateAge() < MAX_BUTTON_STATE_AGE) && (_tsms->read()))
#endif
            {
                // _pedal.brake->getValue() >= 20%  turn  cal led on if _pedal.brake->getValue() >=  40%  turn  LCT LED on

                if (_pedal.brake->getValue() >= BRAKE_START_THRESHHOLD)
                {
                    _motorController->setRFE(MOTOR_CONTROLLER_RFE_ENABLE);

                    _buzzer->playRtdTone();
                    _motorController->setRUN(MOTOR_CONTROLLER_RUN_ENABLE);
                    _setState(DRIVE);
                }
            }
            if (_pedal.brake->getValue() >= BRAKE_START_THRESHHOLD)
            {
                _ledService._led.rtd->setState(LED_ON);
                _ledService._led.rtd->setBlinking(BLINKING_SLOW);
            }

            if (_button.calibrate->getState() == LONG_CLICKED && _button.calibrate->getStateAge() < MAX_BUTTON_STATE_AGE)
            {
                _startCalibratePedals();
                _setState(HV_CALIBRATING);
                _timer.reset();
                _timer.start();
            }
            checkPowerMenu();
            break;
        case DRIVE:
#ifndef NO_ACCU
            if (!(_tsms->read()) || (_motorController->getDcVoltage() < 60) || _button.reset->getState() == PRESSED)
#else
            if (!(_tsms->read()) || _button.reset->getState() == PRESSED)
#endif
            {
                _motorController->setRUN(MOTOR_CONTROLLER_RUN_DISABLE);
                _motorController->setRFE(MOTOR_CONTROLLER_RFE_DISABLE);
                _setState(LV_CALIBRATED);
                return;
            }
            break;
        case CAR_ERROR:
            if (_button.reset->getState() == LONG_CLICKED && _button.reset->getStateAge() < MAX_BUTTON_STATE_AGE && _button.start->getState() == LONG_CLICKED && _button.start->getStateAge() < MAX_BUTTON_STATE_AGE)
            {
                _setState(LV_NOT_CALIBRATED);
            }

        default:
            _setState(CAR_ERROR);
            break;
        }
    }

    void checkPowerMenu()
    {
#ifdef ENABLE_POWER_MENU
        if (_button.reset->getState() == PRESSED && _button.reset->getStateAge() < MAX_BUTTON_STATE_AGE)
        {
            resetWasPressedBefore = true;
        }

        if (_button.reset->getState() == NOT_PRESSED && _button.reset->getStateAge() < MAX_BUTTON_STATE_AGE)
        {
            if (!resetWasNotPressedBefore)
            {
                resetWasPressedBefore = false;
                resetWasNotPressedBefore = true;
            }
            else
            {
                if (resetWasPressedBefore)
                {
                    resetWasPressedBefore = false;

                    if (resetWasNotPressedBefore)
                    {
                        resetWasNotPressedBefore = false;
                        _sycleModes();
                    }
                }
                else
                {
                    resetWasNotPressedBefore = true;
                }
            }
        }
#endif
    }

    void _beepTimes(uint8_t times, float beepOnTime = BEEP_MULTI_BEEP_TIME, float beepOffTime = BEEP_MULTI_SILENT_TIME)
    {
        _buzzer->setBeep(BUZZER_MONO_TONE);
        for (uint8_t i = 0; i < times; i++)
        {
            _buzzer->setState(BUZZER_ON);
            wait_ms(beepOnTime);
            _buzzer->setState(BUZZER_OFF);
            wait_ms(beepOffTime);
        }
    }

    /*
        Modes:
            0: Normal mode. 60kW, 200A max, 230 Nm
            1: Baby mode. 40kW, 120A max, 120 Nm
            2: High Power mode. 80kW, 250A max, 230 Nm
            3: ASR Easy Test. 60kW, 200A max, 180 Nm, ASR On
            4: ASR High Power Test. 80kW, 250A max, 230 Nm, ASR On
    */
    void _sycleModes()
    {
#ifdef ENABLE_POWER_MENU
        _currentModeId++;

#ifdef EXPERIMENTELL_ASR_ACTIVE
        if (_currentModeId > 4)
            _currentModeId = 0;
#else
        if (_currentModeId > 1)
            _currentModeId = 0;
#endif

        // _beepTimes(_currentModeId + 1); Remove Comment for BEEP when switching between powermodes

        switch (_currentModeId)
        {
        case 0:
            maxAllowedPower = MODE_0_ACCU_MAX_ALLOWED_POWER;
            maxAllowedAmpere = MODE_0_ACCU_MAX_ALLOWED_CURRENT;
            maxAllowedTorque = MODE_0_MOTOR_MAX_ALLOWED_TORQUE;

#ifdef EXPERIMENTELL_ASR_ACTIVE
            asrOn = false;
#endif
            break;

        case 1:
            maxAllowedPower = MODE_1_ACCU_MAX_ALLOWED_POWER;
            maxAllowedAmpere = MODE_1_ACCU_MAX_ALLOWED_CURRENT;
            maxAllowedTorque = MODE_1_MOTOR_MAX_ALLOWED_TORQUE;

#ifdef EXPERIMENTELL_ASR_ACTIVE
            asrOn = false;
#endif
            break;

        case 2:
            maxAllowedPower = MODE_2_ACCU_MAX_ALLOWED_POWER;
            maxAllowedAmpere = MODE_2_ACCU_MAX_ALLOWED_CURRENT;
            maxAllowedTorque = MODE_2_MOTOR_MAX_ALLOWED_TORQUE;

#ifdef EXPERIMENTELL_ASR_ACTIVE
            asrOn = false;
#endif
            break;

#ifdef EXPERIMENTELL_ASR_ACTIVE

        case 3:
            maxAllowedPower = MODE_3_ACCU_MAX_ALLOWED_POWER;
            maxAllowedAmpere = MODE_3_ACCU_MAX_ALLOWED_CURRENT;
            maxAllowedTorque = MODE_3_MOTOR_MAX_ALLOWED_TORQUE;

            asrOn = true;
            break;

        case 4:
            maxAllowedPower = MODE_4_ACCU_MAX_ALLOWED_POWER;
            maxAllowedAmpere = MODE_4_ACCU_MAX_ALLOWED_CURRENT;
            maxAllowedTorque = MODE_4_MOTOR_MAX_ALLOWED_TORQUE;

            asrOn = true;
            break;

#endif

        default:
            // What??
            break;
        }
#endif
    }
};

#endif // SCAR_H